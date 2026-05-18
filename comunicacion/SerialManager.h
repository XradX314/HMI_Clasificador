/**
 * @file SerialManager.h
 * @brief Gestión del puerto serie y despacho de tramas del protocolo UNER.
 *
 * SerialManager actúa como fachada entre la interfaz gráfica y el stack
 * de comunicación.  Encapsula la apertura/cierre del QSerialPort, el
 * watchdog de heartbeat y la traducción entre señales Qt y tramas UNER.
 */

#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "UnerProtocol.h"

/**
 * @class SerialManager
 * @brief Gestiona la conexión serie y el ciclo de vida del protocolo UNER.
 *
 * Responsabilidades:
 * - Abrir y cerrar el QSerialPort con los parámetros correctos (8N1).
 * - Reenviar bytes recibidos al @ref UnerProtocol para su decodificación.
 * - Mantener el watchdog de heartbeat (timeout 7 s).
 * - Exponer métodos de envío tipados para cada comando del protocolo.
 * - Emitir señales de alto nivel cuando llegan tramas del MCU.
 */
class SerialManager : public QObject
{
    Q_OBJECT

public:
    /** @brief Velocidad de baudios por defecto. */
    static constexpr qint32 DEFAULT_BAUD         = 9600;

    /** @brief Tiempo máximo sin recibir heartbeat antes de emitir connectionLost (ms). */
    static constexpr int    HEARTBEAT_TIMEOUT_MS = 7000;

    /**
     * @brief Construye el SerialManager.
     * @param parent Objeto padre Qt (o nullptr).
     */
    explicit SerialManager(QObject *parent = nullptr);

    /** @brief Destructor – cierra el puerto si está abierto. */
    ~SerialManager() override;

    // ── Gestión de conexión ────────────────────────────────────

    /**
     * @brief Abre el puerto serie especificado.
     * @param portName Nombre del puerto (ej. "COM3" o "/dev/ttyUSB0").
     * @param baud     Velocidad en baudios (por defecto DEFAULT_BAUD).
     * @return true si el puerto se abrió correctamente, false en caso de error.
     */
    bool   open(const QString &portName, qint32 baud = DEFAULT_BAUD);

    /** @brief Cierra el puerto serie y detiene el watchdog. */
    void   close();

    /**
     * @brief Indica si el puerto está actualmente abierto.
     * @return true si el puerto está abierto.
     */
    bool   isOpen() const;

    /**
     * @brief Nombre del puerto actualmente abierto.
     * @return Nombre del puerto, o cadena vacía si no está conectado.
     */
    QString currentPort() const;

    /**
     * @brief Lista los puertos serie disponibles en el sistema.
     * @return Lista de nombres de puertos (ej. ["COM1", "COM3"]).
     */
    static QStringList availablePorts();

    // ── Comandos de control ────────────────────────────────────

    /**
     * @brief Envía el comando de inicio del clasificador (0x50).
     * @param s0 Tipo de caja para la salida 0.
     * @param s1 Tipo de caja para la salida 1.
     * @param s2 Tipo de caja para la salida 2.
     */
    void sendStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2);

    /** @brief Envía el comando de detención (0x51). */
    void sendStop();

    /** @brief Envía el comando de reset (0x53). */
    void sendReset();

    /**
     * @brief Envía el índice de velocidad de la cinta (0x54).
     * @param velIdx Índice en el rango [1, 10].
     */
    void sendVelocidad(uint8_t velIdx);

    // ── Comandos de configuración (0x60–0x67) ─────────────────

    /**
     * @brief Envía la configuración de modo ciego y distancias (0x60).
     * @param cfg      Modo ciego y distancias S0→salida.
     * @param numBytes Cantidad de bytes del payload (1–4).
     */
    void sendBlindDist(const Uner::CiegoDistancias &cfg, uint8_t numBytes = 4);

    /** @brief Envía un disparo de trigger del HC-SR04 (0x61, sin payload). */
    void sendTrigger();

    /**
     * @brief Envía el ancho de la caja de referencia (0x62).
     * @param anchoCm Ancho en centímetros.
     */
    void sendAnchoCaja(uint8_t anchoCm);

    /**
     * @brief Envía la calibración completa del sensor (0x63).
     * @param cfg Alturas de referencia, tolerancia y tiempos de brazo.
     */
    void sendCalibracion(const Uner::CalibracionCfg &cfg);

    /**
     * @brief Envía la configuración del sensor HC-SR04 (0x64).
     * @param cfg Pulso de trigger y timeout.
     */
    void sendHcsr04Cfg(const Uner::Hcsr04Cfg &cfg);

    /**
     * @brief Envía la configuración del servo SG90 (0x65).
     * @param cfg Período, pulso mínimo, máximo y neutro.
     */
    void sendSg90Cfg(const Uner::Sg90Cfg &cfg);

    /**
     * @brief Envía el tiempo de debounce de los sensores IR (0x66).
     * @param debounce Número de ticks de debounce.
     */
    void sendIrDebounce(uint8_t debounce);

    /**
     * @brief Envía timers generales y alturas de caja (0x67).
     * @param cfg Timers, ángulos de servo y alturas por tipo de caja.
     */
    void sendTimersCfg(const Uner::TimersCfg &cfg);

    /**
     * @brief Envía una trama ya construida directamente al puerto.
     * @param frame Trama UNER completa (resultado de UnerProtocol::encode()).
     */
    void sendRaw(const QByteArray &frame);

signals:
    /**
     * @brief Se emite cuando el puerto serie se conecta exitosamente.
     * @param port Nombre del puerto abierto.
     */
    void connected(const QString &port);

    /** @brief Se emite cuando el puerto serie se cierra correctamente. */
    void disconnected();

    /**
     * @brief Se emite cuando el watchdog de heartbeat expira (7 s sin 0xF0).
     */
    void connectionLost();

    /**
     * @brief Se emite ante un error del QSerialPort.
     * @param msg Descripción del error.
     */
    void errorOccurred(const QString &msg);

    /**
     * @brief Se emite para cada trama decodificada (señal de bajo nivel).
     * @param frame Trama recibida.
     */
    void frameReady(const Uner::Frame &frame);

    /** @brief Se emite cuando se recibe un heartbeat 0xF0 del MCU. */
    void aliveReceived();

    /**
     * @brief Se emite cuando el MCU detecta una caja (0x5F).
     * @param alturaCm Altura medida en centímetros.
     */
    void cajaMedida(uint8_t alturaCm);

    /**
     * @brief Se emite cuando cambia el estado de un sensor IR (0x5E).
     * @param outNum Número de sensor (0–3).
     * @param activo true si el sensor está activo (objeto detectado).
     */
    void sensorIrActualizado(uint8_t outNum, bool activo);

    /**
     * @brief Se emite cuando el MCU activa un brazo actuador (0x52).
     * @param servoIdx Índice del servo actuado (0–2).
     */
    void brazoActuado(uint8_t servoIdx);

private slots:
    /** @brief Slot invocado cuando hay datos disponibles en el puerto. */
    void onDataReady();

    /**
     * @brief Slot invocado ante errores del QSerialPort.
     * @param error Código de error del puerto.
     */
    void onSerialError(QSerialPort::SerialPortError error);

    /** @brief Slot invocado cuando expira el watchdog de heartbeat. */
    void onHeartbeatTimeout();

    /**
     * @brief Slot invocado por UnerProtocol cuando llega una trama decodificada.
     * @param frame Trama decodificada.
     */
    void onFrameReceived(const Uner::Frame &frame);

private:
    QSerialPort  *m_serial          {nullptr}; ///< Puerto serie Qt.
    UnerProtocol *m_protocol        {nullptr}; ///< Parser/encoder del protocolo UNER.
    QTimer       *m_heartbeatTimer  {nullptr}; ///< Watchdog de heartbeat.

    /**
     * @brief Despacha una trama decodificada a la señal apropiada.
     * @param frame Trama a despachar.
     */
    void dispatchFrame(const Uner::Frame &frame);
};
