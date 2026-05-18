/**
 * @file SerialManager.cpp
 * @brief Implementación del gestor de comunicación serie y despachador de tramas UNER.
 */
#include "SerialManager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
    , m_serial        (new QSerialPort(this))
    , m_protocol      (new UnerProtocol(this))
    , m_heartbeatTimer(new QTimer(this))
{
    connect(m_protocol, &UnerProtocol::frameReceived, this, &SerialManager::onFrameReceived);
    connect(m_protocol, &UnerProtocol::checksumError,
            this, [](const QByteArray &r){ qWarning() << "[SM] CKS error:" << r.toHex(' '); });

    connect(m_serial, &QSerialPort::readyRead,      this, &SerialManager::onDataReady);
    connect(m_serial, &QSerialPort::errorOccurred,  this, &SerialManager::onSerialError);

    m_heartbeatTimer->setSingleShot(false);
    m_heartbeatTimer->setInterval(HEARTBEAT_TIMEOUT_MS);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &SerialManager::onHeartbeatTimeout);
}

SerialManager::~SerialManager() { close(); }

bool SerialManager::open(const QString &portName, qint32 baud)
{
    if (m_serial->isOpen()) close();
    m_serial->setPortName(portName);
    m_serial->setBaudRate(baud);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(tr("No se pudo abrir %1: %2").arg(portName, m_serial->errorString()));
        return false;
    }
    m_protocol->reset();
    m_heartbeatTimer->start();
    emit connected(portName);
    qInfo() << "[SM] Abierto:" << portName << "@" << baud;
    return true;
}

void SerialManager::close()
{
    m_heartbeatTimer->stop();
    if (m_serial->isOpen()) { m_serial->close(); emit disconnected(); }
    m_protocol->reset();
}

bool        SerialManager::isOpen()       const { return m_serial->isOpen(); }
QString     SerialManager::currentPort()  const { return m_serial->portName(); }

QStringList SerialManager::availablePorts()
{
    QStringList l;
    for (const QSerialPortInfo &i : QSerialPortInfo::availablePorts()) l << i.portName();
    return l;
}

// ─── Envío ────────────────────────────────────────────────
void SerialManager::sendRaw(const QByteArray &frame)
{
    if (!m_serial->isOpen()) { qWarning() << "[SM] Puerto no abierto."; return; }
    qDebug() << "[SerialManager] TX:" << frame.toHex(' ');
    m_serial->write(frame);
}

// Control
void SerialManager::sendStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2)
{ sendRaw(UnerProtocol::cmdStart(s0, s1, s2)); }

void SerialManager::sendStop()    { sendRaw(UnerProtocol::cmdStop()); }
void SerialManager::sendReset()   { sendRaw(UnerProtocol::cmdReset()); }
void SerialManager::sendVelocidad(uint8_t v) { sendRaw(UnerProtocol::cmdVelocidad(v)); }

// Configuración 0x60–0x67
void SerialManager::sendBlindDist(const Uner::CiegoDistancias &cfg, uint8_t numBytes)
{ sendRaw(UnerProtocol::cmdBlindDist(cfg, numBytes)); }

void SerialManager::sendTrigger()
{ sendRaw(UnerProtocol::cmdTrigger()); }

void SerialManager::sendAnchoCaja(uint8_t anchoCm)
{ sendRaw(UnerProtocol::cmdAnchoCaja(anchoCm)); }

void SerialManager::sendCalibracion(const Uner::CalibracionCfg &cfg)
{ sendRaw(UnerProtocol::cmdCalibracion(cfg)); }

void SerialManager::sendHcsr04Cfg(const Uner::Hcsr04Cfg &cfg)
{ sendRaw(UnerProtocol::cmdHcsr04Cfg(cfg)); }

void SerialManager::sendSg90Cfg(const Uner::Sg90Cfg &cfg)
{ sendRaw(UnerProtocol::cmdSg90Cfg(cfg)); }

void SerialManager::sendIrDebounce(uint8_t debounce)
{ sendRaw(UnerProtocol::cmdIrDebounce(debounce)); }

void SerialManager::sendTimersCfg(const Uner::TimersCfg &cfg)
{ sendRaw(UnerProtocol::cmdTimersCfg(cfg)); }

// ─── Slots privados ───────────────────────────────────────
void SerialManager::onDataReady()
{
    const QByteArray data = m_serial->readAll();
    qDebug() << "[SerialManager] RX:" << data.toHex(' ');
    m_protocol->feed(data);
}

void SerialManager::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;
    const QString msg = m_serial->errorString();
    qWarning() << "[SM] Error:" << msg;
    emit errorOccurred(msg);
    if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError)
        close();
}

void SerialManager::onHeartbeatTimeout()
{
    qWarning() << "[SM] Heartbeat timeout.";
    emit connectionLost();
}

void SerialManager::onFrameReceived(const Uner::Frame &frame)
{
    emit frameReady(frame);
    dispatchFrame(frame);
}

// ─── Dispatch ─────────────────────────────────────────────
void SerialManager::dispatchFrame(const Uner::Frame &frame)
{
    switch (frame.cmd) {

    case Uner::CMD_ALIVE:
        m_heartbeatTimer->start();
        sendRaw(UnerProtocol::ackAlive());
        emit aliveReceived();
        qDebug() << "[SerialManager] Heartbeat recibido → ACK enviado.";
        break;

    // 0x5F: caja medida. payload[0] = altura en cm
    case Uner::CMD_CAJA_DETECT:
        if (!frame.payload.isEmpty()) {
            const uint8_t h = static_cast<uint8_t>(frame.payload.at(0));
            emit cajaMedida(h);
            qDebug() << "[SerialManager] Caja medida:" << h << "cm";
        }
        break;

    // 0x5E: estado sensores IR. pares (outNum, IRState)
    case Uner::CMD_IR_STATE:
        for (int i = 0; i + 1 < frame.payload.size(); i += 2) {
            const uint8_t outNum = static_cast<uint8_t>(frame.payload.at(i));
            const bool    activo = static_cast<uint8_t>(frame.payload.at(i + 1)) != 0;
            emit sensorIrActualizado(outNum, activo);
        }
        break;

    // 0x52: brazo actuado. payload[0]=máscara servo, payload[1]=estado
    case Uner::CMD_BRAZO:
        if (frame.payload.size() >= 2) {
            const uint8_t mask = static_cast<uint8_t>(frame.payload.at(0));
            // El MCU envía la máscara de bits; encontramos el índice
            for (uint8_t i = 0; i < 3; i++) {
                if (mask & (1 << i)) {
                    emit brazoActuado(i);
                    qDebug() << "[SerialManager] Brazo actuado, servo:" << i;
                }
            }
        }
        break;

    default:
        qDebug() << "[SerialManager] Frame no manejado:" << Qt::hex << frame.cmd;
        break;
    }
}
