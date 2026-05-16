#include "VelocidadDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>

static constexpr int TIMEOUT_S = 60;

VelocidadDialog::VelocidadDialog(QWidget *parent)
    : QDialog(parent)
    , m_tickTimer(new QTimer(this))
{
    setWindowTitle("Medición de velocidad de cinta");
    setModal(false);
    setMinimumWidth(340);
    setMaximumWidth(420);

    m_tickTimer->setInterval(1000);
    connect(m_tickTimer, &QTimer::timeout, this, &VelocidadDialog::onTick);

    buildUi();
    applyLightStyle();
}

// =============================================================
//  buildUi
// =============================================================
void VelocidadDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);
    root->setContentsMargins(20, 20, 20, 20);

    // ── Descripción ──────────────────────────────────────────
    auto *desc = new QLabel(
        "Ingresá el ancho de la caja que se usará como referencia "
        "y presioná «Medir velocidad». El MCU medirá el tiempo que "
        "tarda la caja en pasar y devolverá la velocidad en cm/s "
        "(CMD 0x62). Tiempo máximo de espera: 60 segundos.");
    desc->setWordWrap(true);
    desc->setObjectName("descLabel");
    root->addWidget(desc);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("sepLine");
    root->addWidget(sep);

    // ── Entrada ───────────────────────────────────────────────
    auto *gbInput = new QGroupBox("Parámetro de medición");
    auto *gl = new QGridLayout(gbInput);
    gl->setSpacing(10);
    gl->setColumnStretch(1, 1);

    gl->addWidget(new QLabel("Ancho de caja:"), 0, 0);
    m_spinAncho = new QSpinBox;
    m_spinAncho->setRange(1, 50);
    m_spinAncho->setValue(10);
    m_spinAncho->setSuffix(" cm");
    m_spinAncho->setToolTip("Ancho de la caja usada como referencia (payload del 0x62)");
    gl->addWidget(m_spinAncho, 0, 1);

    auto *notaCmd = new QLabel("→ CMD 0x62 payload: uint8_t ancho_cm");
    notaCmd->setObjectName("notaLabel");
    gl->addWidget(notaCmd, 1, 0, 1, 2);

    root->addWidget(gbInput);

    // ── Resultado en grande ───────────────────────────────────
    auto *gbResult = new QGroupBox("Resultado");
    auto *vlResult = new QVBoxLayout(gbResult);
    vlResult->setSpacing(4);

    auto *hlNum = new QHBoxLayout;
    hlNum->setAlignment(Qt::AlignCenter);

    m_lblResultado = new QLabel("–");
    m_lblResultado->setObjectName("lblResultado");
    m_lblResultado->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_lblResultado->setStyleSheet("font-size: 48px; font-weight: 700; color: #1D4ED8;");

    m_lblUnidad = new QLabel("cm/s");
    m_lblUnidad->setObjectName("lblUnidad");
    m_lblUnidad->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_lblUnidad->setStyleSheet("font-size: 18px; color: #6B7280; padding-bottom: 8px;");

    hlNum->addWidget(m_lblResultado);
    hlNum->addSpacing(6);
    hlNum->addWidget(m_lblUnidad);
    vlResult->addLayout(hlNum);

    m_lblEstado = new QLabel("Presioná «Medir velocidad» para iniciar.");
    m_lblEstado->setObjectName("lblEstado");
    m_lblEstado->setAlignment(Qt::AlignCenter);
    m_lblEstado->setWordWrap(true);
    vlResult->addWidget(m_lblEstado);

    // Barra de progreso (cuenta regresiva)
    m_progress = new QProgressBar;
    m_progress->setRange(0, TIMEOUT_S);
    m_progress->setValue(0);
    m_progress->setTextVisible(true);
    m_progress->setFormat("esperando… %v s restantes");
    m_progress->setVisible(false);
    vlResult->addWidget(m_progress);

    root->addWidget(gbResult);

    // ── Botones ───────────────────────────────────────────────
    auto *hl = new QHBoxLayout;
    m_btnCerrar = new QPushButton("Cerrar");
    m_btnMedir  = new QPushButton("▶  Medir velocidad");
    m_btnMedir->setObjectName("btnMedirVel");

    hl->addWidget(m_btnCerrar);
    hl->addStretch();
    hl->addWidget(m_btnMedir);
    root->addLayout(hl);

    connect(m_btnCerrar, &QPushButton::clicked, this, &QDialog::close);
    connect(m_btnMedir,  &QPushButton::clicked, this, &VelocidadDialog::onMedirClicked);
}

// =============================================================
//  Slots
// =============================================================
void VelocidadDialog::onMedirClicked()
{
    const uint8_t ancho = static_cast<uint8_t>(m_spinAncho->value());
    setBusyState(true);
    emit requestMedirVelocidad(ancho);
}

void VelocidadDialog::onTick()
{
    m_segundosRestantes--;
    m_progress->setValue(m_segundosRestantes);
    m_progress->setFormat(QString("esperando… %1 s restantes").arg(m_segundosRestantes));

    if (m_segundosRestantes <= 10)
        m_progress->setStyleSheet("QProgressBar::chunk { background: #DC2626; }");
    else if (m_segundosRestantes <= 30)
        m_progress->setStyleSheet("QProgressBar::chunk { background: #D97706; }");
}

// =============================================================
//  API pública
// =============================================================
void VelocidadDialog::velocidadRecibida(uint8_t velCmS)
{
    m_tickTimer->stop();
    setBusyState(false);

    m_lblResultado->setText(QString::number(velCmS));
    m_lblResultado->setStyleSheet("font-size: 48px; font-weight: 700; color: #16A34A;");
    m_lblEstado->setText(QString("Medición completada. Velocidad: %1 cm/s").arg(velCmS));
    m_lblEstado->setStyleSheet("color: #16A34A; font-weight: 500;");
    m_progress->setVisible(false);
}

void VelocidadDialog::velocidadFallo()
{
    m_tickTimer->stop();
    setBusyState(false);

    m_lblResultado->setText("!");
    m_lblResultado->setStyleSheet("font-size: 48px; font-weight: 700; color: #DC2626;");
    m_lblEstado->setText("Timeout: el MCU no respondió en 60 segundos.");
    m_lblEstado->setStyleSheet("color: #DC2626; font-weight: 500;");
    m_progress->setVisible(false);
}

void VelocidadDialog::setDarkMode(bool dark)
{
    if (dark) applyDarkStyle(); else applyLightStyle();
}

// =============================================================
//  Estado busy
// =============================================================
void VelocidadDialog::setBusyState(bool busy)
{
    m_btnMedir->setEnabled(!busy);
    m_spinAncho->setEnabled(!busy);

    if (busy) {
        m_segundosRestantes = TIMEOUT_S;
        m_progress->setRange(0, TIMEOUT_S);
        m_progress->setValue(TIMEOUT_S);
        m_progress->setFormat(QString("esperando… %1 s restantes").arg(TIMEOUT_S));
        m_progress->setStyleSheet("QProgressBar::chunk { background: #1D4ED8; }");
        m_progress->setVisible(true);
        m_lblResultado->setText("–");
        m_lblResultado->setStyleSheet("font-size: 48px; font-weight: 700; color: #1D4ED8;");
        m_lblEstado->setText("Midiendo… esperando respuesta CMD 0x62 del MCU.");
        m_lblEstado->setStyleSheet("color: #D97706; font-weight: 500;");
        m_tickTimer->start();
    } else {
        m_tickTimer->stop();
    }
}

// =============================================================
//  Temas
// =============================================================
void VelocidadDialog::applyDarkStyle()
{
    setStyleSheet(R"(
        QDialog   { background: #1F2937; }
        QGroupBox { background: #111827; border: 1px solid #374151; border-radius: 8px;
                    color: #9CA3AF; margin-top: 14px; padding-top: 8px; }
        QGroupBox::title { background: #111827; left: 10px; padding: 0 4px; }
        QLabel    { color: #D1D5DB; }
        QLabel#descLabel, QLabel#notaLabel { color: #9CA3AF; font-size: 11px; }
        QLabel#lblEstado  { color: #9CA3AF; font-size: 12px; }
        QLabel#lblUnidad  { color: #6B7280; }
        QSpinBox  { background: #374151; border: 1px solid #4B5563;
                    border-radius: 6px; color: #F9FAFB; padding: 4px 8px; }
        QSpinBox:focus { border-color: #60A5FA; }
        QProgressBar { background: #374151; border: 1px solid #4B5563;
                       border-radius: 4px; color: #F9FAFB; font-size: 11px; text-align: center; }
        QPushButton { background: #374151; border: 1px solid #4B5563;
                      border-radius: 6px; color: #D1D5DB; padding: 6px 14px; min-height: 28px; }
        QPushButton:hover    { background: #4B5563; }
        QPushButton:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
        QPushButton#btnMedirVel { background: #064E3B; border-color: #065F46; color: #6EE7B7; }
        QPushButton#btnMedirVel:hover    { background: #065F46; }
        QPushButton#btnMedirVel:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
    )");
}

void VelocidadDialog::applyLightStyle()
{
    setStyleSheet(R"(
        QDialog   { background: #FFFFFF; }
        QGroupBox { background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px;
                    color: #6B7280; margin-top: 14px; padding-top: 8px; }
        QGroupBox::title { background: #F9FAFB; left: 10px; padding: 0 4px; }
        QLabel    { color: #374151; }
        QLabel#descLabel, QLabel#notaLabel { color: #6B7280; font-size: 11px; }
        QLabel#lblEstado  { color: #6B7280; font-size: 12px; }
        QLabel#lblUnidad  { color: #9CA3AF; }
        QSpinBox  { background: #FFFFFF; border: 1px solid #D1D5DB;
                    border-radius: 6px; color: #111827; padding: 4px 8px; }
        QSpinBox:focus { border-color: #93C5FD; }
        QProgressBar { background: #F3F4F6; border: 1px solid #E5E7EB;
                       border-radius: 4px; color: #374151; font-size: 11px; text-align: center; }
        QPushButton { background: #FFFFFF; border: 1px solid #D1D5DB;
                      border-radius: 6px; color: #374151; padding: 6px 14px; min-height: 28px; }
        QPushButton:hover    { background: #F9FAFB; }
        QPushButton:disabled { background: #F3F4F6; color: #D1D5DB; border-color: #E5E7EB; }
        QPushButton#btnMedirVel { background: #ECFDF5; border-color: #6EE7B7; color: #065F46; }
        QPushButton#btnMedirVel:hover    { background: #D1FAE5; }
        QPushButton#btnMedirVel:disabled { background: #F3F4F6; color: #D1D5DB; border-color: #E5E7EB; }
    )");
}
