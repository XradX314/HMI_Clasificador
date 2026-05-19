#include "CintaVisualizador.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QFontMetrics>

// ─── CintaCanvas ────────────────────────────────────────────────────────────

CintaCanvas::CintaCanvas(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(600, 160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(&m_timer, &QTimer::timeout, this, &CintaCanvas::tick);
    m_timer.start(33); // ~30 fps
    m_elapsed.start();
}

void CintaCanvas::setDistancias(uint8_t d0, uint8_t d1, uint8_t d2)
{
    m_dist[0] = d0;
    m_dist[1] = d1;
    m_dist[2] = d2;
}

void CintaCanvas::setAnchoCaja(uint8_t ancho)
{
    m_anchoCaja = (ancho > 0) ? ancho : 10;
}

void CintaCanvas::onVelocidadActualizada(uint8_t vel)
{
    m_velCmS = static_cast<float>(vel);
}

void CintaCanvas::onCajaMedida(uint8_t alturaCm)
{
    CajaEnCinta c;
    c.posX     = 0.0f;
    c.alturaCm = alturaCm;
    c.tipo     = (alturaCm >= 18) ? 10 :
                 (alturaCm >= 14) ?  8 :
                 (alturaCm >= 10) ?  6 : 0;
    m_cajas.append(c);
}

void CintaCanvas::onSensorIr(uint8_t outNum, bool activo)
{
    if (outNum > 2 || !activo) return;
    float snapX = static_cast<float>(m_dist[outNum]);
    float threshold = static_cast<float>(m_dist[0]) * 0.6f;
    for (auto &c : m_cajas) {
        if (qAbs(c.posX - snapX) < threshold) {
            c.posX = snapX;
            break;
        }
    }
}

void CintaCanvas::onBrazoActuado(uint8_t servoIdx)
{
    if (servoIdx > 2) return;
    m_brazos[servoIdx].activo   = true;
    m_brazos[servoIdx].msQuedan = 400;
}

float CintaCanvas::pixPerCm() const
{
    float totalCm = static_cast<float>(m_dist[2]) + static_cast<float>(m_anchoCaja) * 4.0f + 10.0f;
    return (width() - 60.0f) / totalCm;
}

float CintaCanvas::beltY() const
{
    return height() * 0.58f;
}

QColor CintaCanvas::colorForTipo(uint8_t tipo) const
{
    switch (tipo) {
    case 6:  return QColor("#22c55e"); // verde  – Pequeña
    case 8:  return QColor("#3b82f6"); // azul   – Mediana
    case 10: return QColor("#f59e0b"); // naranja– Grande
    default: return QColor("#9ca3af"); // gris   – desconocida
    }
}

QString CintaCanvas::letraForTipo(uint8_t tipo) const
{
    switch (tipo) {
    case 6:  return "P";
    case 8:  return "M";
    case 10: return "G";
    default: return "?";
    }
}

void CintaCanvas::tick()
{
    const qint64 dt_ms = m_elapsed.restart();
    const float  dt_s  = dt_ms / 1000.0f;
    const float  maxX  = static_cast<float>(m_dist[2]) + static_cast<float>(m_anchoCaja) * 2.0f;

    if (m_velCmS > 0.0f) {
        for (auto &c : m_cajas)
            c.posX += m_velCmS * dt_s;
    }

    // remove boxes that have left the belt
    m_cajas.removeIf([maxX](const CajaEnCinta &c){ return c.posX > maxX; });

    // decrement arm timers
    for (auto &b : m_brazos) {
        if (b.activo) {
            b.msQuedan -= static_cast<int>(dt_ms);
            if (b.msQuedan <= 0) {
                b.activo   = false;
                b.msQuedan = 0;
            }
        }
    }

    update();
}

void CintaCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const float ppc   = pixPerCm();
    const float by    = beltY();
    const float bh    = height() * 0.10f;   // belt height in px
    const float ox    = 30.0f;              // left margin in px

    // ── Belt ─────────────────────────────────────────────────────────────
    const float beltW = static_cast<float>(m_dist[2])
                      + static_cast<float>(m_anchoCaja) * 3.0f + 5.0f;
    QRectF beltRect(ox, by, beltW * ppc, bh);
    p.setBrush(QColor("#374151"));
    p.setPen(Qt::NoPen);
    p.drawRect(beltRect);

    // rollers
    p.setBrush(QColor("#6b7280"));
    const float rollerR = bh * 0.6f;
    p.drawEllipse(QPointF(ox,                     by + bh * 0.5f), rollerR, rollerR);
    p.drawEllipse(QPointF(ox + beltW * ppc,       by + bh * 0.5f), rollerR, rollerR);

    // ── Sensor markers ───────────────────────────────────────────────────
    // IR0 (entry) is at posX = 0 (ox)
    // IR1/2/3 at dist[0/1/2]
    const float sensorXs[4] = {
        ox,
        ox + m_dist[0] * ppc,
        ox + m_dist[1] * ppc,
        ox + m_dist[2] * ppc
    };
    const char *sensorLabels[4] = {"IR0", "IR1", "IR2", "IR3"};

    QPen dashPen(QColor("#a78bfa"), 1, Qt::DashLine);
    p.setFont(QFont("Segoe UI", 7));

    for (int i = 0; i < 4; i++) {
        p.setPen(dashPen);
        p.drawLine(QPointF(sensorXs[i], by - 20.0f),
                   QPointF(sensorXs[i], by + bh + 4.0f));
        p.setPen(QColor("#c4b5fd"));
        p.drawText(QRectF(sensorXs[i] - 15.0f, by - 30.0f, 30.0f, 14.0f),
                   Qt::AlignCenter, sensorLabels[i]);
    }

    // ── Servo arms ───────────────────────────────────────────────────────
    for (int i = 0; i < 3; i++) {
        if (!m_brazos[i].activo) continue;
        const float ax    = ox + m_dist[i] * ppc;
        const float armH  = bh * 1.6f;
        const float armW  = ppc * static_cast<float>(m_anchoCaja) * 0.5f;
        QRectF armRect(ax - armW * 0.5f, by + bh * 0.5f, armW, armH);
        p.setBrush(QColor("#f97316"));
        p.setPen(Qt::NoPen);
        p.drawRect(armRect);
    }

    // ── Boxes ─────────────────────────────────────────────────────────────
    for (const auto &c : m_cajas) {
        const float bw   = m_anchoCaja * ppc;
        const float bx   = ox + c.posX * ppc - bw * 0.5f;
        const float boxH = qMax(bh * 0.8f, static_cast<float>(c.alturaCm) * ppc * 0.4f);
        QRectF boxRect(bx, by - boxH, bw, boxH);

        p.setBrush(colorForTipo(c.tipo));
        p.setPen(Qt::NoPen);
        QPainterPath path;
        path.addRoundedRect(boxRect, 3, 3);
        p.drawPath(path);

        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 8, QFont::Bold));
        p.drawText(boxRect, Qt::AlignCenter, letraForTipo(c.tipo));
    }

    // ── Velocity label ───────────────────────────────────────────────────
    p.setPen(QColor("#d1d5db"));
    p.setFont(QFont("Segoe UI", 8));
    const QString velStr = (m_velCmS > 0.0f)
        ? QString("v = %1 cm/s").arg(static_cast<int>(m_velCmS))
        : QString("v = -- cm/s");
    p.drawText(QRectF(ox, height() - 18.0f, 120.0f, 16.0f), Qt::AlignLeft | Qt::AlignVCenter, velStr);
}

// ─── CintaVisualizador ───────────────────────────────────────────────────────

CintaVisualizador::CintaVisualizador(QWidget *parent)
    : QDialog(parent)
    , m_canvas(new CintaCanvas(this))
{
    setWindowTitle("Visualizador de Cinta");
    setModal(false);
    setMinimumSize(700, 220);
    resize(900, 240);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(m_canvas);
    setLayout(layout);

    applyDarkStyle();
}

void CintaVisualizador::setDarkMode(bool dark)
{
    if (dark) applyDarkStyle();
    else      applyLightStyle();
}

void CintaVisualizador::applyDarkStyle()
{
    setStyleSheet("QDialog { background: #1f2937; }");
    m_canvas->setStyleSheet("background: #111827; border-radius: 6px;");
}

void CintaVisualizador::applyLightStyle()
{
    setStyleSheet("QDialog { background: #f3f4f6; }");
    m_canvas->setStyleSheet("background: #e5e7eb; border-radius: 6px;");
}
