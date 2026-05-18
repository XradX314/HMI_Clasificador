/**
 * @file VelocidadDialog.cpp
 * @brief Implementación del diálogo de ancho de caja de referencia (CMD 0x62).
 */
#include "VelocidadDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>

VelocidadDialog::VelocidadDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Ancho de caja (0x62)");
    setModal(false);
    setMinimumWidth(320);
    setMaximumWidth(400);
    buildUi();
    applyLightStyle();
}

void VelocidadDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);
    root->setContentsMargins(20, 20, 20, 20);

    auto *desc = new QLabel(
        "Ingresá el ancho de la caja de referencia y presioná «Enviar». "
        "El MCU utilizará este valor para calcular internamente la "
        "velocidad de la cinta (CMD 0x62, 1 byte: anchoCaja).");
    desc->setWordWrap(true);
    desc->setObjectName("descLabel");
    root->addWidget(desc);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    root->addWidget(sep);

    auto *gb = new QGroupBox("Parámetro");
    auto *gl = new QGridLayout(gb);
    gl->setSpacing(10);
    gl->setColumnStretch(1, 1);

    gl->addWidget(new QLabel("Ancho de caja:"), 0, 0);
    m_spinAncho = new QSpinBox;
    m_spinAncho->setRange(1, 255);
    m_spinAncho->setValue(10);
    m_spinAncho->setSuffix(" cm");
    m_spinAncho->setToolTip("anchoCaja — payload del CMD 0x62");
    gl->addWidget(m_spinAncho, 0, 1);

    auto *nota = new QLabel("→ CMD 0x62: [anchoCaja]");
    nota->setObjectName("notaLabel");
    gl->addWidget(nota, 1, 0, 1, 2);

    root->addWidget(gb);

    auto *hl = new QHBoxLayout;
    m_btnCerrar = new QPushButton("Cerrar");
    m_btnEnviar = new QPushButton("▶  Enviar 0x62");
    m_btnEnviar->setObjectName("btnEnviarAncho");

    hl->addWidget(m_btnCerrar);
    hl->addStretch();
    hl->addWidget(m_btnEnviar);
    root->addLayout(hl);

    connect(m_btnCerrar, &QPushButton::clicked, this, &QDialog::close);
    connect(m_btnEnviar, &QPushButton::clicked, this, &VelocidadDialog::onEnviarClicked);
}

void VelocidadDialog::onEnviarClicked()
{
    emit requestAnchoCaja(static_cast<uint8_t>(m_spinAncho->value()));
}

void VelocidadDialog::setDarkMode(bool dark)
{
    if (dark) applyDarkStyle(); else applyLightStyle();
}

void VelocidadDialog::applyDarkStyle()
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
                      border-radius: 6px; color: #D1D5DB; padding: 6px 14px; min-height: 28px; }
        QPushButton:hover    { background: #4B5563; }
        QPushButton#btnEnviarAncho { background: #064E3B; border-color: #065F46; color: #6EE7B7; }
        QPushButton#btnEnviarAncho:hover { background: #065F46; }
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
        QSpinBox  { background: #FFFFFF; border: 1px solid #D1D5DB;
                    border-radius: 6px; color: #111827; padding: 4px 8px; }
        QSpinBox:focus { border-color: #93C5FD; }
        QPushButton { background: #FFFFFF; border: 1px solid #D1D5DB;
                      border-radius: 6px; color: #374151; padding: 6px 14px; min-height: 28px; }
        QPushButton:hover    { background: #F9FAFB; }
        QPushButton#btnEnviarAncho { background: #ECFDF5; border-color: #6EE7B7; color: #065F46; }
        QPushButton#btnEnviarAncho:hover { background: #D1FAE5; }
    )");
}
