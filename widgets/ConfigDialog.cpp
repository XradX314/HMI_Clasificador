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
    setMinimumWidth(420);
    buildUi(current);
    applyLightStyle();
}

void ConfigDialog::buildUi(const Uner::CalibracionCfg &c)
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);
    root->setContentsMargins(20, 20, 20, 20);

    auto *desc = new QLabel(
        "Presioná «Medir» junto a cada campo para enviar 0x61 y "
        "recibir la distancia actual del sensor. El valor se completa "
        "automáticamente. Luego ajustá los tiempos de brazo y enviá 0x63.");
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
    glAlt->setColumnMinimumWidth(2, 72);
    glAlt->setColumnMinimumWidth(3, 64);

    auto *hdrMedir     = new QLabel("medir");
    auto *hdrResultado = new QLabel("resultado");
    hdrMedir->setObjectName("descLabel");
    hdrResultado->setObjectName("descLabel");
    hdrMedir->setAlignment(Qt::AlignCenter);
    hdrResultado->setAlignment(Qt::AlignCenter);
    glAlt->addWidget(hdrMedir,     0, 2);
    glAlt->addWidget(hdrResultado, 0, 3);

    makeRow(glAlt, 1, "Distancia piso:",  m_rowPiso,    c.calibracion[0], 1, 200, Piso,
            "Distancia del sensor al piso sin caja");
    makeRow(glAlt, 2, "Caja pequeña:",    m_rowPequenia, c.calibracion[1], 1, 100, Pequenia,
            "Altura de la caja pequeña");
    makeRow(glAlt, 3, "Caja mediana:",    m_rowMediana,  c.calibracion[2], 1, 100, Mediana,
            "Altura de la caja mediana");
    makeRow(glAlt, 4, "Caja grande:",     m_rowGrande,   c.calibracion[3], 1, 100, Grande,
            "Altura de la caja grande");

    glAlt->addWidget(new QLabel("Tolerancia ±:"), 5, 0);
    m_spinTolerancia = new QSpinBox;
    m_spinTolerancia->setRange(0, 20);
    m_spinTolerancia->setValue(c.tolerancia);
    m_spinTolerancia->setSuffix(" cm");
    m_spinTolerancia->setToolTip("Margen de error permitido en la clasificación");
    glAlt->addWidget(m_spinTolerancia, 5, 1);

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
    m_spinArmExtend = makeSpinTick(c.time_arm_extend, "Tiempo de extensión del brazo");
    glArm->addWidget(m_spinArmExtend, 0, 1);

    glArm->addWidget(new QLabel("Retracción:"), 1, 0);
    m_spinArmRetract = makeSpinTick(c.time_arm_retract, "Tiempo de retracción del brazo");
    glArm->addWidget(m_spinArmRetract, 1, 1);

    auto *notaTick = new QLabel("1 tick = 2 ms   →   125 ticks = 250 ms");
    notaTick->setObjectName("notaLabel");
    glArm->addWidget(notaTick, 2, 0, 1, 2);

    root->addWidget(gbArm);

    auto *nota = new QLabel(
        "0x63 payload: [piso, pequeña, mediana, grande, tolerancia, ext, ret]   "
        "0x61: sin payload → respuesta uint8_t cm");
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

void ConfigDialog::makeRow(QGridLayout *gl, int row, const QString &label,
                            Row &r, int val, int minV, int maxV,
                            Field field, const QString &tooltip)
{
    gl->addWidget(new QLabel(label), row, 0);

    r.spin = new QSpinBox;
    r.spin->setRange(minV, maxV);
    r.spin->setValue(val);
    r.spin->setSuffix(" cm");
    r.spin->setToolTip(tooltip);
    gl->addWidget(r.spin, row, 1);

    r.medir = new QPushButton("Medir");
    r.medir->setObjectName("btnMedir");
    r.medir->setFixedWidth(68);
    r.medir->setToolTip("Enviar CMD 0x61 y esperar medición (3 s)");
    connect(r.medir, &QPushButton::clicked, this, [this, field]() {
        setBusyState(field, true);
        emit requestMedir(field);
    });
    gl->addWidget(r.medir, row, 2);

    r.status = new QLabel("–");
    r.status->setObjectName("statusLabel");
    r.status->setAlignment(Qt::AlignCenter);
    gl->addWidget(r.status, row, 3);
}

void ConfigDialog::medicionRecibida(Field field, uint8_t cm)
{
    setBusyState(field, false);
    Row *r = nullptr;
    switch (field) {
    case Piso:     r = &m_rowPiso;     break;
    case Pequenia: r = &m_rowPequenia; break;
    case Mediana:  r = &m_rowMediana;  break;
    case Grande:   r = &m_rowGrande;   break;
    }
    if (!r) return;
    r->spin->setValue(cm);
    r->status->setText(QString("%1 cm").arg(cm));
    r->status->setStyleSheet("color: #16A34A; font-weight: 600; font-size: 12px;");
}

void ConfigDialog::medicionFallo(Field field)
{
    setBusyState(field, false);
    Row *r = nullptr;
    switch (field) {
    case Piso:     r = &m_rowPiso;     break;
    case Pequenia: r = &m_rowPequenia; break;
    case Mediana:  r = &m_rowMediana;  break;
    case Grande:   r = &m_rowGrande;   break;
    }
    if (!r) return;
    r->status->setText("timeout");
    r->status->setStyleSheet("color: #DC2626; font-weight: 600; font-size: 12px;");
}

void ConfigDialog::setBusyState(Field field, bool busy)
{
    Q_UNUSED(field)
    for (Row *r : {&m_rowPiso, &m_rowPequenia, &m_rowMediana, &m_rowGrande})
        r->medir->setEnabled(!busy);
    m_btnAplicar->setEnabled(!busy);
}

Uner::CalibracionCfg ConfigDialog::config() const
{
    Uner::CalibracionCfg cfg;
    cfg.calibracion[0]  = static_cast<uint8_t>(m_rowPiso.spin->value());
    cfg.calibracion[1]  = static_cast<uint8_t>(m_rowPequenia.spin->value());
    cfg.calibracion[2]  = static_cast<uint8_t>(m_rowMediana.spin->value());
    cfg.calibracion[3]  = static_cast<uint8_t>(m_rowGrande.spin->value());
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
        QPushButton#btnMedir { background: #1E3A5F; border-color: #1D4ED8; color: #93C5FD; }
        QPushButton#btnMedir:hover { background: #1D4ED8; }
        QPushButton#btnMedir:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
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
        QPushButton#btnMedir { background: #EFF6FF; border-color: #93C5FD; color: #1D4ED8; }
        QPushButton#btnMedir:hover { background: #DBEAFE; }
        QPushButton#btnMedir:disabled { background: #F3F4F6; color: #D1D5DB; border-color: #E5E7EB; }
    )");
}
