#include "ConfigDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>

ConfigDialog::ConfigDialog(const Uner::ConfigUmbrales &current, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Configuración de umbrales");
    setModal(false);
    setMinimumWidth(380);
    buildUi(current);
    applyLightStyle();
}

// =============================================================
//  buildUi
// =============================================================
void ConfigDialog::buildUi(const Uner::ConfigUmbrales &c)
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);
    root->setContentsMargins(20, 20, 20, 20);

    // ── Descripción ──────────────────────────────────────────
    auto *desc = new QLabel(
        "Presioná «Medir» junto a cada campo para enviar 0x61 y "
        "recibir la distancia actual del sensor. El programa espera "
        "exclusivamente la respuesta antes de procesar otros comandos.");
    desc->setWordWrap(true);
    desc->setObjectName("descLabel");
    root->addWidget(desc);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("sepLine");
    root->addWidget(sep);

    // ── GroupBox con filas ────────────────────────────────────
    auto *gb = new QGroupBox("Parámetros del sensor HC-SR04");
    auto *gl = new QGridLayout(gb);
    gl->setSpacing(8);
    gl->setColumnStretch(1, 1);
    gl->setColumnMinimumWidth(2, 80);   // columna del botón Medir
    gl->setColumnMinimumWidth(3, 60);   // columna de estado

    // Cabecera de columnas
    auto makeHeader = [&](int col, const QString &text) {
        auto *h = new QLabel(text);
        h->setObjectName("headerLabel");
        gl->addWidget(h, 0, col, Qt::AlignCenter);
    };
    makeHeader(1, "cm");
    makeHeader(2, "");
    makeHeader(3, "resultado");

    // Filas de medición
    makeRow(gl, 1, "Distancia piso:",  m_rowPiso,    c.distancia_piso_cm, 1, 200, Piso,
            "Distancia del sensor HC-SR04 al piso sin caja");
    makeRow(gl, 2, "Caja Pequeña:",    m_rowPequenia, c.pequenia_cm,      1, 50,  Pequenia,
            "Altura de la caja pequeña");
    makeRow(gl, 3, "Caja Mediana:",    m_rowMediana,  c.mediana_cm,       1, 50,  Mediana,
            "Altura de la caja mediana");
    makeRow(gl, 4, "Caja Grande:",     m_rowGrande,   c.grande_cm,        1, 50,  Grande,
            "Altura de la caja grande");

    // Tolerancia (sin botón Medir)
    gl->addWidget(new QLabel("Tolerancia ±:"), 5, 0);
    m_spinTolerancia = new QSpinBox;
    m_spinTolerancia->setRange(0, 10);
    m_spinTolerancia->setValue(c.tolerancia_cm);
    m_spinTolerancia->setSuffix(" cm");
    m_spinTolerancia->setToolTip("Margen de error permitido en la clasificación");
    gl->addWidget(m_spinTolerancia, 5, 1);

    root->addWidget(gb);

    // ── Nota de protocolo ─────────────────────────────────────
    auto *nota = new QLabel(
        "0x60 payload: [piso, pequeña, mediana, grande, tolerancia]   "
        "0x61: sin payload → respuesta uint8_t cm");
    nota->setObjectName("notaLabel");
    nota->setWordWrap(true);
    root->addWidget(nota);

    // ── Botones ───────────────────────────────────────────────
    auto *hl = new QHBoxLayout;
    m_btnCerrar  = new QPushButton("Cerrar");
    m_btnAplicar = new QPushButton("Enviar 0x60");
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

// =============================================================
//  makeRow  –  label | spinbox | [Medir] | status
// =============================================================
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

    r.btn = new QPushButton("Medir");
    r.btn->setObjectName("btnMedir");
    r.btn->setFixedWidth(72);
    r.btn->setToolTip("Enviar CMD 0x61 y esperar medición");
    connect(r.btn, &QPushButton::clicked, this, [this, field]() {
        setBusyState(field, true);
        emit requestMedir(field);
    });
    gl->addWidget(r.btn, row, 2);

    r.status = new QLabel("–");
    r.status->setObjectName("statusLabel");
    r.status->setAlignment(Qt::AlignCenter);
    gl->addWidget(r.status, row, 3);
}

// =============================================================
//  API pública
// =============================================================
Uner::ConfigUmbrales ConfigDialog::config() const
{
    return {
        static_cast<uint8_t>(m_rowPiso.spin->value()),
        static_cast<uint8_t>(m_rowPequenia.spin->value()),
        static_cast<uint8_t>(m_rowMediana.spin->value()),
        static_cast<uint8_t>(m_rowGrande.spin->value()),
        static_cast<uint8_t>(m_spinTolerancia->value())
    };
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

    // Actualizar spinbox y mostrar resultado en verde
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

// =============================================================
//  Estado busy mientras espera respuesta
// =============================================================
void ConfigDialog::setBusyState(Field field, bool busy)
{
    m_pendingField = field;

    // Deshabilitar todos los botones Medir mientras se espera
    for (Row *r : {&m_rowPiso, &m_rowPequenia, &m_rowMediana, &m_rowGrande}) {
        r->btn->setEnabled(!busy);
    }
    m_btnAplicar->setEnabled(!busy);

    // Indicar visualmente cuál está esperando
    Row *active = nullptr;
    switch (field) {
    case Piso:     active = &m_rowPiso;     break;
    case Pequenia: active = &m_rowPequenia; break;
    case Mediana:  active = &m_rowMediana;  break;
    case Grande:   active = &m_rowGrande;   break;
    }
    if (active) {
        active->status->setText(busy ? "midiendo…" : active->status->text());
        if (busy)
            active->status->setStyleSheet("color: #D97706; font-size: 12px;");
    }
}

// =============================================================
//  Temas
// =============================================================
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
        QLabel#headerLabel { color: #6B7280; font-size: 10px; font-weight: 500; }
        QLabel#statusLabel { color: #6B7280; font-size: 12px; }
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
        QPushButton#btnMedir { background: #064E3B; border-color: #065F46; color: #6EE7B7; }
        QPushButton#btnMedir:hover    { background: #065F46; }
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
        QLabel#headerLabel { color: #9CA3AF; font-size: 10px; font-weight: 500; }
        QLabel#statusLabel { color: #9CA3AF; font-size: 12px; }
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
        QPushButton#btnMedir { background: #ECFDF5; border-color: #6EE7B7; color: #065F46; }
        QPushButton#btnMedir:hover    { background: #D1FAE5; }
        QPushButton#btnMedir:disabled { background: #F3F4F6; color: #D1D5DB; border-color: #E5E7EB; }
    )");
}
