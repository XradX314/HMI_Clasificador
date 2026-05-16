#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "UnerProtocol.h"

class SerialManager : public QObject
{
    Q_OBJECT

public:
    static constexpr qint32 DEFAULT_BAUD         = 9600;
    static constexpr int    HEARTBEAT_TIMEOUT_MS = 7000;

    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager() override;

    bool   open(const QString &portName, qint32 baud = DEFAULT_BAUD);
    void   close();
    bool   isOpen() const;
    QString currentPort() const;
    static QStringList availablePorts();

    // PC → MCU
    void sendStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2);
    void sendStop();
    void sendReset();
    void sendVelocidad(uint8_t velIdx);
    void sendConfig(const Uner::ConfigUmbrales &cfg);
    void sendMedir();
    void sendMedirVelocidad(uint8_t anchoCm);   // 0x62, espera hasta 60 s
    void sendBlindMode(uint8_t velCmS);         // 0x63, fire & forget
    void sendRaw(const QByteArray &frame);

signals:
    void connected(const QString &port);
    void disconnected();
    void connectionLost();
    void errorOccurred(const QString &msg);
    void frameReady(const Uner::Frame &frame);

    void aliveReceived();
    void cajaMedida(uint8_t alturaCm);
    void sensorIrActualizado(uint8_t outNum, bool activo);
    void brazoActuado(uint8_t servoIdx);
    void medicionLista(uint8_t cm);
    void medicionTimeout();
    void velocidadMedida(uint8_t velCmS);    // respuesta 0x62 del MCU
    void velocidadTimeout();                  // no llegó en 60 s

private slots:
    void onDataReady();
    void onSerialError(QSerialPort::SerialPortError error);
    void onHeartbeatTimeout();
    void onFrameReceived(const Uner::Frame &frame);
    void onMedirTimeout();
    void onVelTimeout();

private:
    QSerialPort  *m_serial          {nullptr};
    UnerProtocol *m_protocol        {nullptr};
    QTimer       *m_heartbeatTimer  {nullptr};
    QTimer       *m_medirTimer      {nullptr};
    QTimer       *m_velTimer        {nullptr};   // timeout 60 s para 0x62
    bool          m_esperandoMedir  {false};
    bool          m_esperandoVel    {false};     // modo exclusivo para 0x62

    void dispatchFrame(const Uner::Frame &frame);
};
