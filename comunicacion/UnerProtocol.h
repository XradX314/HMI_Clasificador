/**
 * @file UnerProtocol.h
 * @brief Definición del protocolo de comunicación serie UNER y el
 *        codificador/decodificador de tramas.
 *
 * El protocolo UNER es un protocolo binario orientado a tramas que
 * opera sobre UART.  Cada trama tiene la estructura:
 *
 * @code
 *   [ 'U' 'N' 'E' 'R' | len | ':' | cmd | payload (n bytes) | checksum ]
 * @endcode
 *
 * donde @p len = 1 + n + 1 y @p checksum es el XOR de todos los bytes
 * anteriores al propio checksum.
 *
 * @version 2.0
 * @author  Proyecto UNER – UNER
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <cstdint>

// ============================================================
//  Comandos del protocolo
// ============================================================

/**
 * @namespace Uner
 * @brief Espacio de nombres con todas las constantes, tipos y
 *        estructuras del protocolo UNER v2.0.
 */
namespace Uner {

/** @brief Heartbeat bidireccional (MCU ↔ PC). */
constexpr uint8_t CMD_ALIVE         = 0xF0;

/** @brief Iniciar clasificador con configuración de 3 salidas. */
constexpr uint8_t CMD_START         = 0x50;

/** @brief Detener el sistema clasificador. */
constexpr uint8_t CMD_STOP          = 0x51;

/** @brief Notificación / ACK de actuación de brazo (bidireccional). */
constexpr uint8_t CMD_BRAZO         = 0x52;

/** @brief Reiniciar el sistema clasificador. */
constexpr uint8_t CMD_RESET         = 0x53;

/** @brief Establecer velocidad de la cinta (índice 1–10, payload = idx×10). */
constexpr uint8_t CMD_VELOCIDAD     = 0x54;

/** @brief Estado de sensores IR (MCU→PC). Pares (outNum, IRState). */
constexpr uint8_t CMD_IR_STATE      = 0x5E;

/** @brief Detección de caja por sensor HC-SR04 (MCU→PC). Payload: altura_cm. */
constexpr uint8_t CMD_CAJA_DETECT   = 0x5F;

/** @brief Activar/desactivar modo ciego y configurar distancias S0→salida (1–4 bytes). */
constexpr uint8_t CMD_BLIND_DIST    = 0x60;

/** @brief Disparar trigger del sensor HC-SR04 (PC→MCU, sin payload). */
constexpr uint8_t CMD_TRIGGER       = 0x61;

/** @brief Informar al MCU el ancho de la caja de referencia para cálculo interno de velocidad. */
constexpr uint8_t CMD_ANCHO_CAJA    = 0x62;

/** @brief Enviar calibración completa: alturas, tolerancia y tiempos de brazo (7 bytes). */
constexpr uint8_t CMD_CALIBRACION   = 0x63;

/** @brief Configurar parámetros del sensor HC-SR04 (3 bytes). */
constexpr uint8_t CMD_HCSR04_CFG    = 0x64;

/** @brief Configurar parámetros del servo SG90 (8 bytes, 4×uint16 hi/lo). */
constexpr uint8_t CMD_SG90_CFG      = 0x65;

/** @brief Configurar tiempo de debounce de los sensores IR (1 byte). */
constexpr uint8_t CMD_IR_DEBOUNCE   = 0x66;

/** @brief Configurar timers y ángulos generales del sistema (8 bytes). */
constexpr uint8_t CMD_TIMERS_CFG    = 0x67;

/** @brief Byte de ACK genérico (0x0D). */
constexpr uint8_t PARAM_ACK         = 0x0D;

/** @brief Byte de ACK para confirmación de brazo retractado (0xFF). */
constexpr uint8_t PARAM_BRAZO_ACK   = 0xFF;

// ============================================================
//  Tipos de caja
// ============================================================

/**
 * @brief Identificador del tipo de caja según su altura medida en cm.
 *
 * Los valores corresponden directamente a la altura real de la caja,
 * por lo que también se usan como payload en CMD_START (salidas[i]).
 */
enum class TipoCaja : uint8_t {
    Ninguna  = 0,   ///< Sin tipo asignado (salida no configurada).
    Pequenia = 6,   ///< Caja pequeña – 6 cm.
    Mediana  = 8,   ///< Caja mediana – 8 cm.
    Grande   = 10   ///< Caja grande  – 10 cm.
};

// ============================================================
//  Trama genérica
// ============================================================

/**
 * @brief Trama decodificada del protocolo UNER.
 */
struct Frame {
    uint8_t    cmd;      ///< Byte de comando (ver constantes CMD_*).
    QByteArray payload;  ///< Payload sin cmd ni checksum.
};

// ============================================================
//  Estructuras de configuración
// ============================================================

/**
 * @brief Datos para el comando 0x60 – Modo ciego y distancias.
 *
 * Puede enviarse con 1 a 4 bytes según cuántos campos se quieran
 * configurar en una sola trama.
 */
struct CiegoDistancias {
    uint8_t modo_ciego {0};       ///< 0 = desactivado, 1 = activado.
    uint8_t dist_s0[3] {30, 60, 90}; ///< Distancias en cm desde S0 a cada salida.
};

/**
 * @brief Datos para el comando 0x63 – Calibración.
 *
 * Payload total: 7 bytes.
 * Orden: calibracion[0..3], tolerancia, time_arm_extend, time_arm_retract.
 */
struct CalibracionCfg {
    uint8_t calibracion[4]   {30, 6, 8, 10}; ///< Distancia al piso, altura pequeña, mediana, grande (cm).
    uint8_t tolerancia        {1};             ///< Margen de error en clasificación (cm).
    uint8_t time_arm_extend   {125};           ///< Tiempo de extensión del brazo (ticks × 2 ms).
    uint8_t time_arm_retract  {125};           ///< Tiempo de retracción del brazo (ticks × 2 ms).
};

/**
 * @brief Datos para el comando 0x64 – Configuración del sensor HC-SR04.
 *
 * Payload total: 3 bytes.
 * Orden: trig_pulse_us, timeout_us[hi], timeout_us[lo].
 */
struct Hcsr04Cfg {
    uint8_t  trig_pulse_us {10};    ///< Duración del pulso de trigger (µs).
    uint16_t timeout_us    {25000}; ///< Tiempo de espera máximo del eco (µs).
};

/**
 * @brief Datos para el comando 0x65 – Configuración del servo SG90.
 *
 * Payload total: 8 bytes (4 × uint16, byte alto primero).
 * Orden: period_us, pulse_min_us, pulse_max_us, pulse_neutral_us.
 */
struct Sg90Cfg {
    uint16_t period_us        {20000}; ///< Período PWM (µs).
    uint16_t pulse_min_us     {600};   ///< Ancho de pulso para 0° (µs).
    uint16_t pulse_max_us     {2400};  ///< Ancho de pulso para 180° (µs).
    uint16_t pulse_neutral_us {1500};  ///< Ancho de pulso para posición neutra (µs).
};

/**
 * @brief Datos para el comando 0x67 – Timers y configuraciones generales.
 *
 * Payload total: 8 bytes.
 * Orden: retract_timer, sg90_angle_detect, sg90_angle_repose,
 *        hcsr_timer, alive_timer, config_cajas[0..2].
 */
struct TimersCfg {
    uint8_t retract_timer     {125};     ///< Timer de retracción del brazo (ticks × 2 ms).
    uint8_t sg90_angle_detect {90};      ///< Ángulo de detección del servo (grados).
    uint8_t sg90_angle_repose {0};       ///< Ángulo de reposo del servo (grados).
    uint8_t hcsr_timer        {25};      ///< Timer de disparo del HC-SR04 (ticks × 2 ms).
    uint8_t alive_timer       {250};     ///< Intervalo de heartbeat (ticks × 2 ms).
    uint8_t config_cajas[3]   {6, 8, 10}; ///< Alturas de referencia de cada tipo de caja (cm).
};

} // namespace Uner


// ============================================================
//  Clase UnerProtocol
// ============================================================

/**
 * @class UnerProtocol
 * @brief Codificador y decodificador de tramas del protocolo UNER.
 *
 * Provee métodos estáticos para construir tramas (encoders) y un
 * parser incremental orientado a bytes para decodificar el stream
 * de datos recibido por el puerto serie (decoder).
 *
 * ## Uso del decoder
 * @code
 *   UnerProtocol proto(parent);
 *   connect(&proto, &UnerProtocol::frameReceived, this, &MiClase::onFrame);
 *   proto.feed(datosRecibidos);
 * @endcode
 *
 * ## Uso de los encoders
 * @code
 *   QByteArray trama = UnerProtocol::cmdStart(
 *       Uner::TipoCaja::Pequenia,
 *       Uner::TipoCaja::Mediana,
 *       Uner::TipoCaja::Grande);
 *   puerto->write(trama);
 * @endcode
 */
class UnerProtocol : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construye el objeto UnerProtocol.
     * @param parent Objeto padre Qt (o nullptr).
     */
    explicit UnerProtocol(QObject *parent = nullptr);

    // ── Encoder genérico ────────────────────────────────────

    /**
     * @brief Construye una trama UNER completa con checksum XOR.
     * @param cmd     Byte de comando.
     * @param payload Payload (puede estar vacío).
     * @return Trama lista para escribir al puerto serie.
     */
    static QByteArray encode(uint8_t cmd, const QByteArray &payload = {});

    // ── Encoders de control ─────────────────────────────────

    /** @brief Genera trama de ACK de heartbeat (0xF0 + 0x0D). */
    static QByteArray ackAlive();

    /**
     * @brief Genera trama de inicio del clasificador (0x50).
     * @param s0 Tipo de caja asignado a la salida 0.
     * @param s1 Tipo de caja asignado a la salida 1.
     * @param s2 Tipo de caja asignado a la salida 2.
     */
    static QByteArray cmdStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2);

    /** @brief Genera trama de detención del sistema (0x51). */
    static QByteArray cmdStop();

    /** @brief Genera trama de reset del sistema (0x53). */
    static QByteArray cmdReset();

    /** @brief Genera trama de ACK de brazo retractado (0x52 + 0xFF). */
    static QByteArray ackBrazo();

    /**
     * @brief Genera trama de velocidad de la cinta (0x54).
     * @param velIdx Índice de velocidad en el rango [1, 10].
     *               El MCU recibe velIdx×10.
     */
    static QByteArray cmdVelocidad(uint8_t velIdx);

    // ── Encoders de configuración (0x60–0x67) ──────────────

    /**
     * @brief Genera trama de modo ciego y distancias (0x60).
     * @param cfg      Estructura con modo_ciego y dist_s0[3].
     * @param numBytes Cantidad de bytes a incluir (1–4).
     *                 - 1: solo modo_ciego.
     *                 - 2–4: incluye distancias progressivamente.
     */
    static QByteArray cmdBlindDist(const Uner::CiegoDistancias &cfg, uint8_t numBytes = 4);

    /**
     * @brief Genera trama de disparo de trigger HC-SR04 (0x61).
     * Sin payload.
     */
    static QByteArray cmdTrigger();

    /**
     * @brief Genera trama de ancho de caja de referencia (0x62).
     * @param anchoCm Ancho de la caja en centímetros.
     */
    static QByteArray cmdAnchoCaja(uint8_t anchoCm);

    /**
     * @brief Genera trama de calibración completa (0x63).
     * @param cfg Estructura con 4 alturas, tolerancia y tiempos de brazo.
     */
    static QByteArray cmdCalibracion(const Uner::CalibracionCfg &cfg);

    /**
     * @brief Genera trama de configuración del sensor HC-SR04 (0x64).
     * @param cfg Estructura con pulso de trigger y timeout.
     */
    static QByteArray cmdHcsr04Cfg(const Uner::Hcsr04Cfg &cfg);

    /**
     * @brief Genera trama de configuración del servo SG90 (0x65).
     * @param cfg Estructura con período, pulso mínimo, máximo y neutro.
     */
    static QByteArray cmdSg90Cfg(const Uner::Sg90Cfg &cfg);

    /**
     * @brief Genera trama de debounce IR (0x66).
     * @param debounce Número de ticks de debounce.
     */
    static QByteArray cmdIrDebounce(uint8_t debounce);

    /**
     * @brief Genera trama de timers y configuraciones generales (0x67).
     * @param cfg Estructura con timers, ángulos de servo y alturas de caja.
     */
    static QByteArray cmdTimersCfg(const Uner::TimersCfg &cfg);

    // ── Decoder ─────────────────────────────────────────────

    /**
     * @brief Alimenta el parser con bytes recibidos del puerto serie.
     *
     * Se puede llamar con cualquier cantidad de bytes.  Cuando se
     * completa y verifica una trama emite la señal @ref frameReceived.
     * @param data Bytes recibidos (puede ser parcial o múltiples tramas).
     */
    void feed(const QByteArray &data);

    /**
     * @brief Reinicia el estado interno del parser.
     *
     * Útil después de abrir el puerto o ante errores de comunicación.
     */
    void reset();

signals:
    /**
     * @brief Se emite cuando se recibe y verifica una trama completa.
     * @param frame Trama decodificada con cmd y payload.
     */
    void frameReceived(const Uner::Frame &frame);

    /**
     * @brief Se emite cuando el checksum de una trama no coincide.
     * @param raw Bytes crudos de la trama con error.
     */
    void checksumError(const QByteArray &raw);

private:
    /**
     * @brief Estados de la máquina de estados del decodificador.
     */
    enum class ParseState : uint8_t {
        WaitU = 0,     ///< Esperando el byte 'U' del encabezado.
        WaitN,         ///< Esperando el byte 'N'.
        WaitE,         ///< Esperando el byte 'E'.
        WaitR,         ///< Esperando el byte 'R'.
        ReadLen,       ///< Leyendo el byte de longitud.
        WaitColon,     ///< Esperando el separador ':'.
        ReadPayload    ///< Leyendo el payload + checksum.
    };

    ParseState m_state     {ParseState::WaitU}; ///< Estado actual del parser.
    uint8_t    m_len       {0};                 ///< Longitud total esperada (cmd + payload + cks).
    uint8_t    m_remaining {0};                 ///< Bytes restantes por leer.
    uint8_t    m_cks       {0};                 ///< Acumulador de checksum XOR.
    QByteArray m_payload;                       ///< Buffer interno de payload.
    QByteArray m_rawFrame;                      ///< Buffer de la trama cruda (diagnóstico).

    /**
     * @brief Procesa el payload interno cuando la trama está completa.
     *
     * Extrae cmd y payload, luego emite @ref frameReceived.
     */
    void processPayload();
};
