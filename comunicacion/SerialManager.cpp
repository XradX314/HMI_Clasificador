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

    m_medirTimer = new QTimer(this);
    m_medirTimer->setSingleShot(true);
    m_medirTimer->setInterval(3000);
    connect(m_medirTimer, &QTimer::timeout, this, &SerialManager::onMedirTimeout);

    m_velTimer = new QTimer(this);
    m_velTimer->setSingleShot(true);
    m_velTimer->setInterval(60000);   // 60 s para medición de velocidad
    connect(m_velTimer, &QTimer::timeout, this, &SerialManager::onVelTimeout);
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

void SerialManager::sendStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2)
{ sendRaw(UnerProtocol::cmdStart(s0, s1, s2)); }

void SerialManager::sendStop()             { sendRaw(UnerProtocol::cmdStop()); }
void SerialManager::sendReset()            { sendRaw(UnerProtocol::cmdReset()); }
void SerialManager::sendVelocidad(uint8_t v) { sendRaw(UnerProtocol::cmdVelocidad(v)); }
void SerialManager::sendConfig(const Uner::ConfigUmbrales &cfg) { sendRaw(UnerProtocol::cmdConfig(cfg)); }

void SerialManager::sendMedir()
{
    if (!m_serial->isOpen()) return;
    m_esperandoMedir = true;
    m_medirTimer->start();
    sendRaw(UnerProtocol::cmdMedir());
    qDebug() << "[SM] CMD_MEDIR enviado — modo exclusivo activo.";
}

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

void SerialManager::sendMedirVelocidad(uint8_t anchoCm)
{
    if (!m_serial->isOpen()) return;
    m_esperandoVel = true;
    m_velTimer->start();
    sendRaw(UnerProtocol::cmdMedirVelocidad(anchoCm));
    qDebug() << "[SM] CMD_MEDIR_VEL enviado, ancho=" << anchoCm << "cm — esperando 0x62.";
}

void SerialManager::sendBlindMode(uint8_t velCmS)
{
    sendRaw(UnerProtocol::cmdBlind(velCmS));
}

void SerialManager::onMedirTimeout()
{
    m_esperandoMedir = false;
    qWarning() << "[SM] CMD_MEDIR timeout — no llegó respuesta 0x61.";
    emit medicionTimeout();
}

void SerialManager::onVelTimeout()
{
    m_esperandoVel = false;
    qWarning() << "[SM] CMD_MEDIR_VEL timeout (60 s).";
    emit velocidadTimeout();
}

// ─── Dispatch ─────────────────────────────────────────────
void SerialManager::dispatchFrame(const Uner::Frame &frame)
{
    // ── Modo exclusivo: esperando 0x61 ────────────────────
    if (m_esperandoMedir) {
        if (frame.cmd == Uner::CMD_MEDIR) {
            m_medirTimer->stop();
            m_esperandoMedir = false;
            const uint8_t cm = frame.payload.isEmpty()
                                ? 0 : static_cast<uint8_t>(frame.payload.at(0));
            qDebug() << "[SM] Medición recibida:" << cm << "cm";
            emit medicionLista(cm);
        } else {
            qDebug() << "[SM] Frame ignorado (esperando 0x61):" << Qt::hex << frame.cmd;
        }
        return;
    }

    // ── Modo exclusivo: esperando 0x62 ────────────────────
    if (m_esperandoVel) {
        if (frame.cmd == Uner::CMD_MEDIR_VEL) {
            m_velTimer->stop();
            m_esperandoVel = false;
            const uint8_t vel = frame.payload.isEmpty()
                                 ? 0 : static_cast<uint8_t>(frame.payload.at(0));
            qDebug() << "[SM] Velocidad medida:" << vel << "cm/s";
            emit velocidadMedida(vel);
        } else {
            qDebug() << "[SM] Frame ignorado (esperando 0x62):" << Qt::hex << frame.cmd;
        }
        return;
    }
    switch (frame.cmd) {

    case Uner::CMD_ALIVE:
        m_heartbeatTimer->start();
        sendRaw(UnerProtocol::ackAlive());
        emit aliveReceived();
        qDebug() << "[SerialManager] Heartbeat recibido → ACK enviado.";
        break;

    // 0x5F: caja medida. payload[0] = altura en cm (6, 8 o 10)
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

    // 0x52: brazo actuado. payload[0]=servoIdx, payload[1]=0x00
    case Uner::CMD_BRAZO:
        if (frame.payload.size() >= 2) {
            const uint8_t servoIdx = static_cast<uint8_t>(frame.payload.at(0));
            if (servoIdx < 3) {
                emit brazoActuado(servoIdx);
                qDebug() << "[SerialManager] Brazo actuado, servo:" << servoIdx;
            } else {
                qWarning() << "[SM] CMD_BRAZO: índice inválido:" << servoIdx;
            }
        }
        break;

    default:
        qDebug() << "[SerialManager] Frame no manejado:" << Qt::hex << frame.cmd;
        break;
    }
}
