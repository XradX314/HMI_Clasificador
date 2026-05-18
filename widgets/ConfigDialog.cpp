/**
 * @file ConfigDialog.cpp
 * @brief Implementación del diálogo de calibración del clasificador (CMD 0x63).
 */
#include "ConfigDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>

ConfigDialog::ConfigDialog(const Uner::CalibracionCfg &current, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Calibración del clasificador (0x63)");
    setModal(false);
    setMinimumWidth(360);
    buildUi(current);
    applyLightStyle();
}

void ConfigDialog::buildUi(const Uner::CalibracionCfg &c)
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);
    root->setContentsMargins(20, 20, 20, 20);

    auto *desc = new QLabel(
        "Configura las alturas de referencia del sensor HC-SR04, "
        "la tolerancia de clasificación y los tiempos de actuación "
        "de los brazos. Se envía como CMD 0x63 (7 bytes).");
    desc->setWordWrap(true);
    desc->setObjectName("descLabel");
    root->addWidget(desc);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    root->addWidget(sep);

    // ── Alturas ───────────────────────────────────────────────
    auto *gbAlt = new QGroupBox("Alturas de referencia (cm)");
    auto *glAlt = new QGridLayout(gbAlt);
    glAlt->setSpacing(8);
    glAlt->setColumnStretch(1, 1);

    auto makeSpinCm = [](int val, int minV, int maxV, const QString &tip) {
        auto *s = new QSpinBox;
        s->setRange(minV, maxV);
        s->setValue(val);
        s->setSuffix(" cm");
        s->setToolTip(tip);
        return s;
    };

    glAlt->addWidget(new QLabel("Distancia piso:"),  0, 0);
    m_spinPiso = makeSpinCm(c.calibracion[0], 1, 200,
                            "Distancia del sensor al piso sin caja (calibracion[0])");
    glAlt->addWidget(m_spinPiso, 0, 1);

    glAlt->addWidget(new QLabel("Caja pequeña:"), 1, 0);
    m_spinPequenia = makeSpinCm(c.calibracion[1], 1, 100,
                                "Altura de la caja pequeña (calibracion[1])");
    glAlt->addWidget(m_spinPequenia, 1, 1);

    glAlt->addWidget(new QLabel("Caja mediana:"), 2, 0);
    m_spinMediana = makeSpinCm(c.calibracion[2], 1, 100,
                               "Altura de la caja mediana (calibracion[2])");
    glAlt->addWidget(m_spinMediana, 2, 1);

    glAlt->addWidget(new QLabel("Caja grande:"), 3, 0);
    m_spinGrande = makeSpinCm(c.calibracion[3], 1, 100,
                              "Altura de la caja grande (calibracion[3])");
    glAlt->addWidget(m_spinGrande, 3, 1);

    glAlt->addWidget(new QLabel("Tolerancia ±:"), 4, 0);
    m_spinTolerancia = makeSpinCm(c.tolerancia, 0, 20,
                                  "Margen de error permitido en la clasificación");
    glAlt->addWidget(m_spinTolerancia, 4, 1);

    root->addWidget(gbAlt);

    // ── Tiempos de brazo ──────────────────────────────────────
    auto *gbArm = new QGroupBox("Tiempos de brazo (ticks × 2 ms)");
    auto *glArm = new QGridLayout(gbArm);
    glArm->setSpacing(8);
    glArm->setColumnStretch(1, 1);

    auto makeSpinTick = [](int val, const QString &tip) {
        auto *s = new QSpinBox;
        s->setRange(0, 255);
        s->setValue(val);
        s->setSuffix(" ticks");
        s->setToolTip(tip);
        return s;
    };

    glArm->addWidget(new QLabel("Extensión:"), 0, 0);
    m_spinArmExtend = makeSpinTick(c.time_arm_extend,
                                   "Tiempo de extensión del brazo (time_arm_extend)");
    glArm->addWidget(m_spinArmExtend, 0, 1);

    glArm->addWidget(new QLabel("Retracción:"), 1, 0);
    m_spinArmRetract = makeSpinTick(c.time_arm_retract,
                                    "Tiempo de retracción del brazo (time_arm_retract)");
    glArm->addWidget(m_spinArmRetract, 1, 1);

    auto *notaTick = new QLabel("1 tick = 2 ms   →   125 ticks = 250 ms");
    notaTick->setObjectName("notaLabel");
    glArm->addWidget(notaTick, 2, 0, 1, 2);

    root->addWidget(gbArm);

    // ── Nota de protocolo ─────────────────────────────────────
    auto *nota = new QLabel(
        "0x63 payload: [piso, pequeña, mediana, grande, tolerancia, ext, ret]");
    nota->setObjectName("notaLabel");
    nota->setWordWrap(true);
    root->addWidget(nota);

    // ── Botones ───────────────────────────────────────────────
    auto *hl = new QHBoxLayout;
    m_btnCerrar  = new QPushButton("Cerrar");
    m_btnAplicar = new QPushButton("Enviar 0x63");
    m_btnAplicar->setObjectName("btnAplicarConfig");

    hl->addWidget(m_btnCerrar);
    hl->addStretch();
    hl->addWidget(m_btnAplicar);
    root->addLayout(hl);

    connect(m_btnCerrar,  &QPushButton::clicked, this, &QDialog::close);
    connect(m_btnAplicar, &QPushButton::clicked, this, [this]() {
        emit configApplied(config());
    });
}

Uner::CalibracionCfg ConfigDialog::config() const
{
    Uner::CalibracionCfg cfg;
    cfg.calibracion[0]  = static_cast<uint8_t>(m_spinPiso->value());
    cfg.calibracion[1]  = static_cast<uint8_t>(m_spinPequenia->value());
    cfg.calibracion[2]  = static_cast<uint8_t>(m_spinMediana->value());
    cfg.calibracion[3]  = static_cast<uint8_t>(m_spinGrande->value());
    cfg.tolerancia       = static_cast<uint8_t>(m_spinTolerancia->value());
    cfg.time_arm_extend  = static_cast<uint8_t>(m_spinArmExtend->value());
    cfg.time_arm_retract = static_cast<uint8_t>(m_spinArmRetract->value());
    return cfg;
}

void ConfigDialog::setDarkMode(bool dark)
{
    if (dark) applyDarkStyle(); else applyLightStyle();
}

void ConfigDialog::applyDarkStyle()
{
    setStyleSheet(R"(
        QDialog   { background: #1F2937; }
        QGroupBox { background: #111827; border: 1px solid #374151; border-radius: 8px;
                    color: #9CA3AF; margin-top: 14px; padding-top: 8px; }
        QGroupBox::title { background: #111827; left: 10px; padding: 0 4px; }
        QLabel    { color: #D1D5DB; }
        QLabel#descLabel, QLabel#notaLabel { color: #9CA3AF; font-size: 11px; }
        QSpinBox  { background: #374151; border: 1px solid #4B5563;
                    border-radius: 6px; color: #F9FAFB; padding: 4px 8px; }
        QSpinBox:focus { border-color: #60A5FA; }
        QPushButton { background: #374151; border: 1px solid #4B5563;
                      border-radius: 6px; color: #D1D5DB;
                      padding: 5px 10px; min-height: 26px; }
        QPushButton:hover    { background: #4B5563; }
        QPushButton:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
        QPushButton#btnAplicarConfig { background: #1D4ED8; border-color: #3B82F6; color: #FFF; }
        QPushButton#btnAplicarConfig:hover { background: #2563EB; }
        QPushButton#btnAplicarConfig:disabled { background: #1F2937; color: #4B5563; }
    )");
}

void ConfigDialog::applyLightStyle()
{
    setStyleSheet(R"(
        QDialog   { background: #FFFFFF; }
        QGroupBox { background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px;
                    color: #6B7280; margin-top: 14px; padding-top: 8px; }
        QGroupBox::title { background: #F9FAFB; left: 10px; padding: 0 4px; }
        QLabel    { color: #374151; }
        QLabel#descLabel, QLabel#notaLabel { color: #6B7280; font-size: 11px; }
        QSpinBox  { background: #FFFFFF; border: 1px solid #D1D5DB;
                    border-radius: 6px; color: #111827; padding: 4px 8px; }
        QSpinBox:focus { border-color: #93C5FD; }
        QPushButton { background: #FFFFFF; border: 1px solid #D1D5DB;
                      border-radius: 6px; color: #374151;
                      padding: 5px 10px; min-height: 26px; }
        QPushButton:hover    { background: #F9FAFB; }
        QPushButton:disabled { background: #F3F4F6; color: #D1D5DB; border-color: #E5E7EB; }
        QPushButton#btnAplicarConfig { background: #EFF6FF; border-color: #93C5FD; color: #1D4ED8; }
        QPushButton#btnAplicarConfig:hover { background: #DBEAFE; }
        QPushButton#btnAplicarConfig:disabled { background: #F3F4F6; color: #D1D5DB; }
    )");
}
