#include "UnerProtocol.h"
#include <QDebug>

static const QByteArray HEADER = QByteArrayLiteral("UNER");
static constexpr char   TOKEN  = ':';

UnerProtocol::UnerProtocol(QObject *parent) : QObject(parent) {}

// ─── Encoder ──────────────────────────────────────────────
QByteArray UnerProtocol::encode(uint8_t cmd, const QByteArray &payload)
{
    const uint8_t n   = static_cast<uint8_t>(payload.size());
    const uint8_t len = static_cast<uint8_t>(1 + n + 1);

    QByteArray frame;
    frame.reserve(4 + 1 + 1 + 1 + n + 1);
    frame.append(HEADER);
    frame.append(static_cast<char>(len));
    frame.append(TOKEN);
    frame.append(static_cast<char>(cmd));
    frame.append(payload);

    uint8_t cks = 0;
    for (unsigned char b : frame) cks ^= b;
    frame.append(static_cast<char>(cks));
    return frame;
}

QByteArray UnerProtocol::ackAlive()
{
    QByteArray p; p.append(static_cast<char>(Uner::PARAM_ACK));
    return encode(Uner::CMD_ALIVE, p);
}

QByteArray UnerProtocol::cmdStart(Uner::TipoCaja s0, Uner::TipoCaja s1, Uner::TipoCaja s2)
{
    QByteArray p;
    p.append(static_cast<char>(0x00));
    p.append(static_cast<char>(static_cast<uint8_t>(s0)));
    p.append(static_cast<char>(static_cast<uint8_t>(s1)));
    p.append(static_cast<char>(static_cast<uint8_t>(s2)));
    return encode(Uner::CMD_START, p);
}

QByteArray UnerProtocol::cmdStop()
{
    QByteArray p; p.append(static_cast<char>(Uner::PARAM_ACK));
    return encode(Uner::CMD_STOP, p);
}

QByteArray UnerProtocol::cmdReset()
{
    QByteArray p; p.append(static_cast<char>(Uner::PARAM_ACK));
    return encode(Uner::CMD_RESET, p);
}

QByteArray UnerProtocol::ackBrazo()
{
    QByteArray p; p.append(static_cast<char>(Uner::PARAM_BRAZO_ACK));
    return encode(Uner::CMD_BRAZO, p);
}

QByteArray UnerProtocol::cmdVelocidad(uint8_t velIdx)
{
    if (velIdx < 1)  velIdx = 1;
    if (velIdx > 10) velIdx = 10;
    QByteArray p; p.append(static_cast<char>(velIdx * 10));
    return encode(Uner::CMD_VELOCIDAD, p);
}

QByteArray UnerProtocol::cmdConfig(const Uner::ConfigUmbrales &cfg)
{
    // CMD 0x60: [distancia_piso, pequeña, mediana, grande, tolerancia]
    QByteArray p;
    p.append(static_cast<char>(cfg.distancia_piso_cm));
    p.append(static_cast<char>(cfg.pequenia_cm));
    p.append(static_cast<char>(cfg.mediana_cm));
    p.append(static_cast<char>(cfg.grande_cm));
    p.append(static_cast<char>(cfg.tolerancia_cm));
    return encode(Uner::CMD_CONFIG, p);
}

QByteArray UnerProtocol::cmdMedir()
{
    return encode(Uner::CMD_MEDIR);
}

QByteArray UnerProtocol::cmdMedirVelocidad(uint8_t anchoCm)
{
    QByteArray p;
    p.append(static_cast<char>(anchoCm));
    return encode(Uner::CMD_MEDIR_VEL, p);
}

QByteArray UnerProtocol::cmdBlind(uint8_t velCmS)
{
    QByteArray p;
    p.append(static_cast<char>(velCmS));
    return encode(Uner::CMD_BLIND, p);
}

// ─── Decoder (state machine) ──────────────────────────────
void UnerProtocol::reset()
{
    m_state     = ParseState::WaitU;
    m_len       = 0; m_remaining = 0; m_cks = 0;
    m_payload.clear(); m_rawFrame.clear();
}

void UnerProtocol::feed(const QByteArray &data)
{
    for (const char ch : data) {
        const uint8_t b = static_cast<uint8_t>(ch);
        switch (m_state) {
        case ParseState::WaitU:
            if (b == 'U') { m_rawFrame.clear(); m_rawFrame.append(ch); m_cks = b; m_state = ParseState::WaitN; }
            break;
        case ParseState::WaitN:
            if (b == 'N') { m_rawFrame.append(ch); m_cks ^= b; m_state = ParseState::WaitE; }
            else           { reset(); if (b=='U'){m_rawFrame.append(ch);m_cks=b;m_state=ParseState::WaitN;} }
            break;
        case ParseState::WaitE:
            if (b == 'E') { m_rawFrame.append(ch); m_cks ^= b; m_state = ParseState::WaitR; }
            else           { reset(); }
            break;
        case ParseState::WaitR:
            if (b == 'R') { m_rawFrame.append(ch); m_cks ^= b; m_state = ParseState::ReadLen; }
            else           { reset(); }
            break;
        case ParseState::ReadLen:
            m_len = b; m_remaining = b;
            m_rawFrame.append(ch); m_cks ^= b;
            m_payload.clear();
            m_state = ParseState::WaitColon;
            break;
        case ParseState::WaitColon:
            if (b == static_cast<uint8_t>(TOKEN)) { m_rawFrame.append(ch); m_cks ^= b; m_state = ParseState::ReadPayload; }
            else { reset(); }
            break;
        case ParseState::ReadPayload:
            m_rawFrame.append(ch);
            if (m_remaining > 1) {
                m_cks ^= b;
                m_payload.append(ch);
                m_remaining--;
            } else {
                if (m_cks == b) processPayload();
                else { qWarning() << "[UNER] CKS error:" << m_rawFrame.toHex(' '); emit checksumError(m_rawFrame); }
                reset();
            }
            break;
        }
    }
}

void UnerProtocol::processPayload()
{
    if (m_payload.isEmpty()) return;
    Uner::Frame f;
    f.cmd     = static_cast<uint8_t>(m_payload.at(0));
    f.payload = m_payload.mid(1);
    emit frameReceived(f);
}
