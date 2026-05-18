/**
 * @file LedAliveWidget.h
 * @brief Widget visual de indicador de heartbeat para el protocolo UNER (CMD 0xF0).
 */

#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QDateTime>

/**
 * @class LedAliveWidget
 * @brief LED circular animado que muestra el estado del heartbeat con el MCU.
 *
 * Recibe eventos de heartbeat mediante @ref onAlive() y actualiza visualmente:
 * - **Verde (On):** heartbeat recibido recientemente (1 s de destello).
 * - **Gris (Off):** sin actividad reciente.
 * - **Naranja (Timeout):** sin heartbeat durante más de 7 s.
 *
 * El widget también muestra etiquetas de texto con la cantidad de heartbeats
 * recibidos, el intervalo estimado y la hora del último pulso.
 */
class LedAliveWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief Construye el widget e inicializa los timers internos. */
    explicit LedAliveWidget(QWidget *parent = nullptr);

    /** @brief Registra un heartbeat recibido y actualiza el estado visual. */
    void onAlive();

    /** @brief Reinicia el widget al estado inicial (offline). */
    void reset();

    /**
     * @brief Aplica el tema claro u oscuro al widget.
     * @param dark true para tema oscuro.
     */
    void setDarkMode(bool dark);

protected:
    /** @brief Dibuja el LED circular en la zona superior del widget. */
    void paintEvent(QPaintEvent *) override;

    /** @brief Tamaño preferido del widget. */
    QSize sizeHint() const override;

    /** @brief Tamaño mínimo del widget. */
    QSize minimumSizeHint() const override;

private slots:
    /** @brief Apaga el LED (transición On → Off) al expirar el timer de destello. */
    void turnOff();

    /** @brief Pone el LED en estado Timeout al expirar el watchdog. */
    void onWatchdogTimeout();

private:
    /** @brief Estado interno del LED. */
    enum class LedState { Off, On, Timeout };

    LedState  m_state   {LedState::Off}; ///< Estado visual actual.
    bool      m_dark    {false};          ///< true si el tema oscuro está activo.

    QTimer   *m_offTimer  {nullptr}; ///< Timer de 1 s para apagar el LED.
    QTimer   *m_watchdog  {nullptr}; ///< Watchdog de 7 s para detectar timeout.

    int       m_count   {0};   ///< Cantidad de heartbeats recibidos en la sesión.
    qint64    m_lastTs  {0};   ///< Timestamp del último heartbeat (ms desde epoch).

    QLabel   *m_labelStatus   {nullptr}; ///< Etiqueta "online / offline / timeout".
    QLabel   *m_labelCount    {nullptr}; ///< Etiqueta "recibidos: N".
    QLabel   *m_labelInterval {nullptr}; ///< Etiqueta "intervalo: N s".
    QLabel   *m_labelLast     {nullptr}; ///< Etiqueta "último: HH:MM:SS".

    static constexpr int LED_D   = 36; ///< Diámetro del LED en píxeles.
    static constexpr int LED_PAD = 8;  ///< Margen vertical del área del LED.

    /** @brief Construye el layout interno (LED + etiquetas). */
    void buildLayout();

    /** @brief Reaplica los estilos CSS de las etiquetas según el tema activo. */
    void refreshLabelStyles();

    /**
     * @brief Actualiza las etiquetas de texto con estadísticas del heartbeat.
     * @param intervalMs Intervalo desde el último heartbeat (-1 si es el primero).
     */
    void updateLabels(qint64 intervalMs);
};
