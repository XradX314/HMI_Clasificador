#pragma once

#include <QObject>
#include <QByteArray>
#include <cstdint>

// ============================================================
//  Protocolo UNER – SimuCinta v1.0.1.1
//
//  MCU → PC:  0xF0 Alive | 0x5E IR State | 0x5F Caja medida
//             0x50/51/52/53/54 ACKs del simulador
//  PC  → MCU: 0xF0 ACK | 0x50 Start | 0x51 Stop | 0x52 Brazo
//             0x53 Reset | 0x54 Velocidad | 0x60 Config umbrales
//             0x63 Modo ciego (toggle, payload: vel_cm_s)
// ============================================================

namespace Uner {

constexpr uint8_t CMD_ALIVE       = 0xF0;
constexpr uint8_t CMD_START       = 0x50;
constexpr uint8_t CMD_STOP        = 0x51;
constexpr uint8_t CMD_BRAZO       = 0x52;
constexpr uint8_t CMD_RESET       = 0x53;
constexpr uint8_t CMD_VELOCIDAD   = 0x54;
constexpr uint8_t CMD_IR_STATE    = 0x5E;
constexpr uint8_t CMD_CAJA_DETECT = 0x5F;
constexpr uint8_t CMD_CONFIG      = 0x60;
constexpr uint8_t CMD_MEDIR       = 0x61;   // PC→MCU: sin payload | MCU→PC: uint8_t cm
constexpr uint8_t CMD_MEDIR_VEL   = 0x62;   // PC→MCU: uint8_t ancho_cm | MCU→PC: uint8_t vel_cm_s
constexpr uint8_t CMD_BLIND       = 0x63;   // PC→MCU: uint8_t vel_cm_s (activa/desactiva modo ciego)

constexpr uint8_t PARAM_ACK       = 0x0D;
constexpr uint8_t PARAM_BRAZO_ACK = 0xFF;
constexpr uint8_t PARAM_RESET_ERR = 0x0A;

// Tipo de caja identificado por su altura en cm
enum class TipoCaja : uint8_t {
    Ninguna  = 0,
    Pequenia = 6,
    Mediana  = 8,
    Grande   = 10
};

struct Frame {
    uint8_t    cmd;
    QByteArray payload;
};

struct ConfigUmbrales {
    uint8_t distancia_piso_cm {30};   // distancia del sensor al piso (referencia)
    uint8_t pequenia_cm       {6};
    uint8_t mediana_cm        {8};
    uint8_t grande_cm         {10};
    uint8_t tolerancia_cm     {1};
};

} // namespace Uner


class UnerProtocol : public QObject
{
    Q_OBJECT

public:
    explicit UnerProtocol(QObject *parent = nullptr);

    static QByteArray encode(uint8_t cmd, const QByteArray &payload = {});

    static QByteArray ackAlive();
    static QByteArray cmdStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2);
    static QByteArray cmdStop();
    static QByteArray cmdReset();
    static QByteArray ackBrazo();
    static QByteArray cmdVelocidad(uint8_t velIdx);
    static QByteArray cmdConfig(const Uner::ConfigUmbrales &cfg);
    static QByteArray cmdMedir();
    static QByteArray cmdMedirVelocidad(uint8_t anchoCm);  // 0x62
    static QByteArray cmdBlind(uint8_t velCmS);            // 0x63

    void feed(const QByteArray &data);
    void reset();

signals:
    void frameReceived(const Uner::Frame &frame);
    void checksumError(const QByteArray &raw);

private:
    enum class ParseState : uint8_t {
        WaitU=0, WaitN, WaitE, WaitR, ReadLen, WaitColon, ReadPayload
    };
    ParseState m_state     {ParseState::WaitU};
    uint8_t    m_len       {0};
    uint8_t    m_remaining {0};
    uint8_t    m_cks       {0};
    QByteArray m_payload;
    QByteArray m_rawFrame;

    void processPayload();
};
