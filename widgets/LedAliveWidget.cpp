#include "LedAliveWidget.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QDateTime>

static constexpr int LED_ON_MS          = 1000;
static constexpr int WATCHDOG_TIMEOUT_MS = 7000;

static const QColor COLOR_OFF     ("#9CA3AF");
static const QColor COLOR_ON      ("#22C55E");
static const QColor COLOR_TIMEOUT ("#F97316");

// =============================================================
//  Constructor
// =============================================================
LedAliveWidget::LedAliveWidget(QWidget *parent)
    : QWidget(parent)
    , m_offTimer (new QTimer(this))
    , m_watchdog (new QTimer(this))
{
    m_offTimer->setSingleShot(true);
    m_offTimer->setInterval(LED_ON_MS);
    connect(m_offTimer, &QTimer::timeout, this, &LedAliveWidget::turnOff);

    m_watchdog->setSingleShot(true);
    m_watchdog->setInterval(WATCHDOG_TIMEOUT_MS);
    connect(m_watchdog, &QTimer::timeout, this, &LedAliveWidget::onWatchdogTimeout);

    buildLayout();
}

// =============================================================
//  buildLayout  –  VBox: spacer(LED_D+2*LED_PAD) | labels
// =============================================================
void LedAliveWidget::buildLayout()
{
    // El paintEvent dibuja el LED en la zona superior del widget.
    // Reservamos ese espacio con un QWidget invisible de altura fija,
    // y ponemos los labels debajo en un QVBoxLayout.

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(4);

    // ── Espacio reservado para el paintEvent del LED ──────────
    auto *ledSpacer = new QWidget;
    ledSpacer->setFixedHeight(LED_D + 2 * LED_PAD);
    ledSpacer->setAttribute(Qt::WA_TransparentForMouseEvents);
    root->addWidget(ledSpacer, 0, Qt::AlignHCenter);

    // ── Labels de estado (debajo del LED) ─────────────────────
    m_labelStatus = new QLabel("offline");
    m_labelStatus->setAlignment(Qt::AlignCenter);

    m_labelCount    = new QLabel("recibidos: 0");
    m_labelInterval = new QLabel("intervalo: –");
    m_labelLast     = new QLabel("último: –");

    for (QLabel *l : {m_labelCount, m_labelInterval, m_labelLast})
        l->setAlignment(Qt::AlignCenter);

    root->addWidget(m_labelStatus);
    root->addWidget(m_labelCount);
    root->addWidget(m_labelInterval);
    root->addWidget(m_labelLast);

    refreshLabelStyles();
}

// =============================================================
//  Painting  –  dibuja solo en la zona superior (LED_D + pad)
// =============================================================
QSize LedAliveWidget::sizeHint()        const { return {130, LED_D + 2*LED_PAD + 80}; }
QSize LedAliveWidget::minimumSizeHint() const { return sizeHint(); }

void LedAliveWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Centro horizontal, zona superior
    const qreal cx = width() / 2.0;
    const qreal cy = LED_PAD + LED_D / 2.0;
    const QRectF rect(cx - LED_D/2.0, cy - LED_D/2.0, LED_D, LED_D);

    QColor fill;
    switch (m_state) {
    case LedState::On:      fill = COLOR_ON;      break;
    case LedState::Timeout: fill = COLOR_TIMEOUT; break;
    default:                fill = m_dark ? QColor("#4B5563") : COLOR_OFF; break;
    }

    p.setPen(fill.darker(130));
    p.setBrush(fill);
    p.drawEllipse(rect);

    if (m_state == LedState::On) {
        QRectF shine(rect.left() + rect.width()*0.25,
                     rect.top()  + rect.height()*0.15,
                     rect.width()*0.30, rect.height()*0.20);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255,255,255,80));
        p.drawEllipse(shine);
    }
}

// =============================================================
//  API pública
// =============================================================
void LedAliveWidget::onAlive()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 iv  = (m_lastTs > 0) ? (now - m_lastTs) : -1;
    m_lastTs = now;
    m_count++;

    m_state = LedState::On;
    m_offTimer->start();
    m_watchdog->start();

    update();
    updateLabels(iv);
}

void LedAliveWidget::reset()
{
    m_offTimer->stop();
    m_watchdog->stop();
    m_state  = LedState::Off;
    m_count  = 0;
    m_lastTs = 0;
    update();
    updateLabels(-1);
    m_labelStatus->setText("offline");
    refreshLabelStyles();
}

void LedAliveWidget::setDarkMode(bool dark)
{
    m_dark = dark;
    update();
    refreshLabelStyles();
}

// =============================================================
//  Slots privados
// =============================================================
void LedAliveWidget::turnOff()
{
    if (m_state == LedState::On) { m_state = LedState::Off; update(); }
}

void LedAliveWidget::onWatchdogTimeout()
{
    m_state = LedState::Timeout;
    update();
    m_labelStatus->setText("timeout");
    m_labelStatus->setStyleSheet("color: #F97316; font-weight: 600; font-size: 12px;");
}

// =============================================================
//  Helpers
// =============================================================
void LedAliveWidget::refreshLabelStyles()
{
    const QString secColor = m_dark ? "#9CA3AF" : "#6B7280";
    const QString dimColor = m_dark ? "#6B7280" : "#9CA3AF";

    m_labelStatus->setStyleSheet(
        QString("color: %1; font-weight: 500; font-size: 12px;").arg(secColor));

    for (QLabel *l : {m_labelCount, m_labelInterval, m_labelLast})
        l->setStyleSheet(QString("color: %1; font-size: 11px;").arg(dimColor));
}

void LedAliveWidget::updateLabels(qint64 intervalMs)
{
    m_labelCount->setText(QString("recibidos: %1").arg(m_count));

    if (intervalMs >= 0)
        m_labelInterval->setText(
            QString("intervalo: %1 s").arg(intervalMs / 1000.0, 0, 'f', 2));
    else
        m_labelInterval->setText("intervalo: –");

    if (m_lastTs > 0)
        m_labelLast->setText("último: " +
            QDateTime::fromMSecsSinceEpoch(m_lastTs).toString("hh:mm:ss.zzz"));

    if (m_state != LedState::Timeout) {
        const bool online = m_count > 0;
        m_labelStatus->setText(online ? "online" : "offline");
        m_labelStatus->setStyleSheet(
            online ? "color: #16A34A; font-weight: 600; font-size: 12px;"
                   : QString("color: %1; font-weight: 500; font-size: 12px;")
                         .arg(m_dark ? "#9CA3AF" : "#6B7280"));
    }
}
