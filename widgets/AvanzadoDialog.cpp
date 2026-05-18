/**
 * @file AvanzadoDialog.cpp
 * @brief Implementación del diálogo de configuración avanzada de hardware (CMDs 0x64–0x67).
 */
#include "AvanzadoDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>

AvanzadoDialog::AvanzadoDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Configuración avanzada");
    setModal(false);
    setMinimumWidth(400);
    buildUi();
    applyLightStyle();
}

void AvanzadoDialog::buildUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget;
    auto *root = new QVBoxLayout(container);
    root->setSpacing(14);
    root->setContentsMargins(16, 16, 16, 16);

    // ── Helpers ───────────────────────────────────────────────
    auto makeU8Spin = [](int val, const QString &tip) {
        auto *s = new QSpinBox;
        s->setRange(0, 255);
        s->setValue(val);
        s->setToolTip(tip);
        return s;
    };
    auto makeU16Spin = [](int val, const QString &tip) {
        auto *s = new QSpinBox;
        s->setRange(0, 65535);
        s->setValue(val);
        s->setToolTip(tip);
        return s;
    };
    auto makeSendBtn = [](const QString &label, const QString &objectName) {
        auto *b = new QPushButton(label);
        b->setObjectName(objectName);
        return b;
    };

    // ── 0x64 – HC-SR04 ────────────────────────────────────────
    {
        auto *gb = new QGroupBox("HC-SR04  (CMD 0x64)");
        auto *gl = new QGridLayout(gb);
        gl->setSpacing(8);
        gl->setColumnStretch(1, 1);

        gl->addWidget(new QLabel("Pulso trigger (µs):"), 0, 0);
        m_spinTrigPulse = makeU8Spin(10, "HCSR04_TRIG_PULSE_US");
        gl->addWidget(m_spinTrigPulse, 0, 1);

        gl->addWidget(new QLabel("Timeout (µs):"), 1, 0);
        m_spinTimeout = makeU16Spin(25000,
            "Timeout de espera del eco (uint16, bytes hi/lo)");
        gl->addWidget(m_spinTimeout, 1, 1);

        auto *nota = new QLabel("Payload: [TRIG_PULSE_US, timeout_hi, timeout_lo]");
        nota->setObjectName("notaLabel");
        gl->addWidget(nota, 2, 0, 1, 2);

        m_btnHcsr04 = makeSendBtn("Enviar 0x64", "btnEnviarAvanz");
        gl->addWidget(m_btnHcsr04, 3, 0, 1, 2);

        connect(m_btnHcsr04, &QPushButton::clicked, this, [this]() {
            Uner::Hcsr04Cfg cfg;
            cfg.trig_pulse_us = static_cast<uint8_t>(m_spinTrigPulse->value());
            cfg.timeout_us    = static_cast<uint16_t>(m_spinTimeout->value());
            emit sendHcsr04(cfg);
        });

        root->addWidget(gb);
    }

    // ── 0x65 – SG90 ───────────────────────────────────────────
    {
        auto *gb = new QGroupBox("Servo SG90  (CMD 0x65)");
        auto *gl = new QGridLayout(gb);
        gl->setSpacing(8);
        gl->setColumnStretch(1, 1);

        gl->addWidget(new QLabel("Período (µs):"), 0, 0);
        m_spinPeriod = makeU16Spin(20000, "SG90_PERIOD_US");
        gl->addWidget(m_spinPeriod, 0, 1);

        gl->addWidget(new QLabel("Pulso mínimo (µs):"), 1, 0);
        m_spinPulseMin = makeU16Spin(600, "SG90_PULSE_MIN_US");
        gl->addWidget(m_spinPulseMin, 1, 1);

        gl->addWidget(new QLabel("Pulso máximo (µs):"), 2, 0);
        m_spinPulseMax = makeU16Spin(2400, "SG90_PULSE_MAX_US");
        gl->addWidget(m_spinPulseMax, 2, 1);

        gl->addWidget(new QLabel("Pulso neutro (µs):"), 3, 0);
        m_spinPulseNeutral = makeU16Spin(1500, "SG90_PULSE_NEUTRAL_US");
        gl->addWidget(m_spinPulseNeutral, 3, 1);

        auto *nota = new QLabel(
            "Payload: 4×uint16 (hi/lo): [period, min, max, neutral]");
        nota->setObjectName("notaLabel");
        gl->addWidget(nota, 4, 0, 1, 2);

        m_btnSg90 = makeSendBtn("Enviar 0x65", "btnEnviarAvanz");
        gl->addWidget(m_btnSg90, 5, 0, 1, 2);

        connect(m_btnSg90, &QPushButton::clicked, this, [this]() {
            Uner::Sg90Cfg cfg;
            cfg.period_us        = static_cast<uint16_t>(m_spinPeriod->value());
            cfg.pulse_min_us     = static_cast<uint16_t>(m_spinPulseMin->value());
            cfg.pulse_max_us     = static_cast<uint16_t>(m_spinPulseMax->value());
            cfg.pulse_neutral_us = static_cast<uint16_t>(m_spinPulseNeutral->value());
            emit sendSg90(cfg);
        });

        root->addWidget(gb);
    }

    // ── 0x66 – IR Debounce ────────────────────────────────────
    {
        auto *gb = new QGroupBox("Debounce IR  (CMD 0x66)");
        auto *gl = new QGridLayout(gb);
        gl->setSpacing(8);
        gl->setColumnStretch(1, 1);

        gl->addWidget(new QLabel("Debounce ticks:"), 0, 0);
        m_spinDebounce = makeU8Spin(5, "IR_DEBOUNCE_TICKS");
        gl->addWidget(m_spinDebounce, 0, 1);

        auto *nota = new QLabel("Payload: [IR_DEBOUNCE_TICKS]");
        nota->setObjectName("notaLabel");
        gl->addWidget(nota, 1, 0, 1, 2);

        m_btnDebounce = makeSendBtn("Enviar 0x66", "btnEnviarAvanz");
        gl->addWidget(m_btnDebounce, 2, 0, 1, 2);

        connect(m_btnDebounce, &QPushButton::clicked, this, [this]() {
            emit sendIrDebounce(static_cast<uint8_t>(m_spinDebounce->value()));
        });

        root->addWidget(gb);
    }

    // ── 0x67 – Timers y configuraciones ──────────────────────
    {
        auto *gb = new QGroupBox("Timers y configuraciones  (CMD 0x67)");
        auto *gl = new QGridLayout(gb);
        gl->setSpacing(8);
        gl->setColumnStretch(1, 1);

        auto addRow = [&](int row, const QString &label, QSpinBox *&spin,
                          int val, const QString &tip) {
            gl->addWidget(new QLabel(label), row, 0);
            spin = makeU8Spin(val, tip);
            gl->addWidget(spin, row, 1);
        };

        addRow(0, "retractTimer:",     m_spinRetract,   125, "retractTimer");
        addRow(1, "SG90_ANGLE_DETECT:",m_spinAngleDet,  90,  "SG90_ANGLE_DETECT");
        addRow(2, "SG90_ANGLE_REPOSE:",m_spinAngleRep,  0,   "SG90_ANGLE_REPOSE");
        addRow(3, "hcsrTimer:",        m_spinHcsrTimer, 25,  "hcsrTimer");
        addRow(4, "aliveTimer:",       m_spinAliveTimer,250, "aliveTimer");

        const char *cajasLabel[3] = {"configCajas[0]:", "configCajas[1]:", "configCajas[2]:"};
        const int   cajasDefault[3] = {6, 8, 10};
        for (int i = 0; i < 3; i++) {
            gl->addWidget(new QLabel(cajasLabel[i]), 5 + i, 0);
            m_spinCajas[i] = makeU8Spin(cajasDefault[i],
                                        QString("configCajas[%1]").arg(i));
            gl->addWidget(m_spinCajas[i], 5 + i, 1);
        }

        auto *nota = new QLabel(
            "Payload: [retract, angleDet, angleRep, hcsr, alive, cajas0, cajas1, cajas2]");
        nota->setObjectName("notaLabel");
        nota->setWordWrap(true);
        gl->addWidget(nota, 8, 0, 1, 2);

        m_btnTimers = makeSendBtn("Enviar 0x67", "btnEnviarAvanz");
        gl->addWidget(m_btnTimers, 9, 0, 1, 2);

        connect(m_btnTimers, &QPushButton::clicked, this, [this]() {
            Uner::TimersCfg cfg;
            cfg.retract_timer     = static_cast<uint8_t>(m_spinRetract->value());
            cfg.sg90_angle_detect = static_cast<uint8_t>(m_spinAngleDet->value());
            cfg.sg90_angle_repose = static_cast<uint8_t>(m_spinAngleRep->value());
            cfg.hcsr_timer        = static_cast<uint8_t>(m_spinHcsrTimer->value());
            cfg.alive_timer       = static_cast<uint8_t>(m_spinAliveTimer->value());
            for (int i = 0; i < 3; i++)
                cfg.config_cajas[i] = static_cast<uint8_t>(m_spinCajas[i]->value());
            emit sendTimers(cfg);
        });

        root->addWidget(gb);
    }

    root->addStretch();
    scroll->setWidget(container);
    outerLayout->addWidget(scroll);

    // Botón cerrar fuera del scroll
    auto *hl = new QHBoxLayout;
    hl->setContentsMargins(16, 8, 16, 16);
    hl->addStretch();
    auto *btnClose = new QPushButton("Cerrar");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::close);
    hl->addWidget(btnClose);
    outerLayout->addLayout(hl);
}

void AvanzadoDialog::setDarkMode(bool dark)
{
    if (dark) applyDarkStyle(); else applyLightStyle();
}

void AvanzadoDialog::applyDarkStyle()
{
    setStyleSheet(R"(
        QDialog, QWidget { background: #1F2937; }
        QScrollArea { background: #1F2937; border: none; }
        QGroupBox { background: #111827; border: 1px solid #374151; border-radius: 8px;
                    color: #9CA3AF; margin-top: 14px; padding-top: 8px; }
        QGroupBox::title { background: #111827; left: 10px; padding: 0 4px; }
        QLabel    { color: #D1D5DB; }
        QLabel#notaLabel { color: #9CA3AF; font-size: 11px; }
        QSpinBox  { background: #374151; border: 1px solid #4B5563;
                    border-radius: 6px; color: #F9FAFB; padding: 4px 8px; }
        QSpinBox:focus { border-color: #60A5FA; }
        QPushButton { background: #374151; border: 1px solid #4B5563;
                      border-radius: 6px; color: #D1D5DB;
                      padding: 5px 10px; min-height: 26px; }
        QPushButton:hover    { background: #4B5563; }
        QPushButton#btnEnviarAvanz { background: #1E3A5F; border-color: #1D4ED8; color: #93C5FD; }
        QPushButton#btnEnviarAvanz:hover { background: #1D4ED8; color: #FFF; }
    )");
}

void AvanzadoDialog::applyLightStyle()
{
    setStyleSheet(R"(
        QDialog, QWidget { background: #FFFFFF; }
        QScrollArea { background: #FFFFFF; border: none; }
        QGroupBox { background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px;
                    color: #6B7280; margin-top: 14px; padding-top: 8px; }
        QGroupBox::title { background: #F9FAFB; left: 10px; padding: 0 4px; }
        QLabel    { color: #374151; }
        QLabel#notaLabel { color: #6B7280; font-size: 11px; }
        QSpinBox  { background: #FFFFFF; border: 1px solid #D1D5DB;
                    border-radius: 6px; color: #111827; padding: 4px 8px; }
        QSpinBox:focus { border-color: #93C5FD; }
        QPushButton { background: #FFFFFF; border: 1px solid #D1D5DB;
                      border-radius: 6px; color: #374151;
                      padding: 5px 10px; min-height: 26px; }
        QPushButton:hover    { background: #F9FAFB; }
        QPushButton#btnEnviarAvanz { background: #EFF6FF; border-color: #93C5FD; color: #1D4ED8; }
        QPushButton#btnEnviarAvanz:hover { background: #DBEAFE; }
    )");
}
