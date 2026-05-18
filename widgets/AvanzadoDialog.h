/**
 * @file AvanzadoDialog.h
 * @brief Diálogo de configuración avanzada de hardware (CMDs 0x64–0x67).
 */

#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include "comunicacion/UnerProtocol.h"

/**
 * @class AvanzadoDialog
 * @brief Diálogo flotante con cuatro secciones de configuración de bajo nivel.
 *
 * Cada sección tiene su propio botón «Enviar» y emite una señal
 * independiente para que MainWindow pueda reenviar el comando al MCU:
 *
 * | Sección       | CMD  | Señal emitida      |
 * |---------------|------|--------------------|
 * | HC-SR04       | 0x64 | sendHcsr04()       |
 * | Servo SG90    | 0x65 | sendSg90()         |
 * | Debounce IR   | 0x66 | sendIrDebounce()   |
 * | Timers/Cajas  | 0x67 | sendTimers()       |
 *
 * El diálogo es desplazable verticalmente para acomodar todos los campos.
 */
class AvanzadoDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construye el diálogo con valores predeterminados.
     * @param parent Objeto padre Qt (o nullptr).
     */
    explicit AvanzadoDialog(QWidget *parent = nullptr);

    /**
     * @brief Aplica el tema claro u oscuro al diálogo.
     * @param dark true para tema oscuro.
     */
    void setDarkMode(bool dark);

signals:
    /**
     * @brief Se emite al presionar «Enviar 0x64» (HC-SR04).
     * @param cfg Configuración del sensor ultrasónico.
     */
    void sendHcsr04(const Uner::Hcsr04Cfg &cfg);

    /**
     * @brief Se emite al presionar «Enviar 0x65» (SG90).
     * @param cfg Configuración del servo.
     */
    void sendSg90(const Uner::Sg90Cfg &cfg);

    /**
     * @brief Se emite al presionar «Enviar 0x66» (debounce IR).
     * @param debounce Número de ticks de debounce.
     */
    void sendIrDebounce(uint8_t debounce);

    /**
     * @brief Se emite al presionar «Enviar 0x67» (timers y cajas).
     * @param cfg Configuración de timers y alturas de caja.
     */
    void sendTimers(const Uner::TimersCfg &cfg);

private:
    // ── Sección 0x64 – HC-SR04 ───────────────────────────────
    QSpinBox    *m_spinTrigPulse   {nullptr}; ///< HCSR04_TRIG_PULSE_US (uint8).
    QSpinBox    *m_spinTimeout     {nullptr}; ///< Timeout del eco (uint16, µs).
    QPushButton *m_btnHcsr04       {nullptr}; ///< Botón «Enviar 0x64».

    // ── Sección 0x65 – SG90 ──────────────────────────────────
    QSpinBox    *m_spinPeriod      {nullptr}; ///< SG90_PERIOD_US.
    QSpinBox    *m_spinPulseMin    {nullptr}; ///< SG90_PULSE_MIN_US.
    QSpinBox    *m_spinPulseMax    {nullptr}; ///< SG90_PULSE_MAX_US.
    QSpinBox    *m_spinPulseNeutral{nullptr}; ///< SG90_PULSE_NEUTRAL_US.
    QPushButton *m_btnSg90         {nullptr}; ///< Botón «Enviar 0x65».

    // ── Sección 0x66 – IR Debounce ────────────────────────────
    QSpinBox    *m_spinDebounce    {nullptr}; ///< IR_DEBOUNCE_TICKS (uint8).
    QPushButton *m_btnDebounce     {nullptr}; ///< Botón «Enviar 0x66».

    // ── Sección 0x67 – Timers / Cajas ────────────────────────
    QSpinBox    *m_spinRetract     {nullptr}; ///< retractTimer.
    QSpinBox    *m_spinAngleDet    {nullptr}; ///< SG90_ANGLE_DETECT.
    QSpinBox    *m_spinAngleRep    {nullptr}; ///< SG90_ANGLE_REPOSE.
    QSpinBox    *m_spinHcsrTimer   {nullptr}; ///< hcsrTimer.
    QSpinBox    *m_spinAliveTimer  {nullptr}; ///< aliveTimer.
    QSpinBox    *m_spinCajas[3]    {nullptr, nullptr, nullptr}; ///< configCajas[0..2].
    QPushButton *m_btnTimers       {nullptr}; ///< Botón «Enviar 0x67».

    /** @brief Construye toda la interfaz del diálogo. */
    void buildUi();

    /** @brief Aplica la hoja de estilo del tema oscuro. */
    void applyDarkStyle();

    /** @brief Aplica la hoja de estilo del tema claro. */
    void applyLightStyle();
};
