/**
 * @file UnerProtocol.cpp
 * @brief Implementación del codificador y decodificador del protocolo UNER.
 *
 * Todos los encoders generan tramas con la estructura:
 * @code
 *   UNER | len | ':' | cmd | payload | checksum_XOR
 * @endcode
 *
 * El decoder implementa una máquina de estados que consume bytes de
 * a uno y emite frameReceived() cuando verifica la trama completa.
 */
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

// ─── Comandos de control ──────────────────────────────────
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

// ─── Nuevos comandos 0x60–0x67 ───────────────────────────

// 0x60: modo_ciego [, dist_s0_a_salida[0], [1], [2]]
// numBytes controla cuántos bytes se incluyen (1 a 4).
QByteArray UnerProtocol::cmdBlindDist(const Uner::CiegoDistancias &cfg, uint8_t numBytes)
{
    if (numBytes < 1) numBytes = 1;
    if (numBytes > 4) numBytes = 4;
    QByteArray p;
    p.append(static_cast<char>(cfg.modo_ciego));
    if (numBytes >= 2) p.append(static_cast<char>(cfg.dist_s0[0]));
    if (numBytes >= 3) p.append(static_cast<char>(cfg.dist_s0[1]));
    if (numBytes >= 4) p.append(static_cast<char>(cfg.dist_s0[2]));
    return encode(Uner::CMD_BLIND_DIST, p);
}

// 0x61: sin datos
QByteArray UnerProtocol::cmdTrigger()
{
    return encode(Uner::CMD_TRIGGER);
}

// 0x62: anchoCaja (1 byte)
QByteArray UnerProtocol::cmdAnchoCaja(uint8_t anchoCm)
{
    QByteArray p; p.append(static_cast<char>(anchoCm));
    return encode(Uner::CMD_ANCHO_CAJA, p);
}

// 0x63: calibracion[4] + tolerancia + time_arm_extend + time_arm_retract
QByteArray UnerProtocol::cmdCalibracion(const Uner::CalibracionCfg &cfg)
{
    QByteArray p;
    for (int i = 0; i < 4; i++) p.append(static_cast<char>(cfg.calibracion[i]));
    p.append(static_cast<char>(cfg.tolerancia));
    p.append(static_cast<char>(cfg.time_arm_extend));
    p.append(static_cast<char>(cfg.time_arm_retract));
    return encode(Uner::CMD_CALIBRACION, p);
}

// 0x64: HCSR04_TRIG_PULSE_US + timeout[hi, lo]
QByteArray UnerProtocol::cmdHcsr04Cfg(const Uner::Hcsr04Cfg &cfg)
{
    QByteArray p;
    p.append(static_cast<char>(cfg.trig_pulse_us));
    p.append(static_cast<char>(cfg.timeout_us >> 8));
    p.append(static_cast<char>(cfg.timeout_us & 0xFF));
    return encode(Uner::CMD_HCSR04_CFG, p);
}

// 0x65: SG90_PERIOD_US, SG90_PULSE_MIN_US, SG90_PULSE_MAX_US, SG90_PULSE_NEUTRAL_US
//       cada uno como 2 bytes (hi, lo)
QByteArray UnerProtocol::cmdSg90Cfg(const Uner::Sg90Cfg &cfg)
{
    QByteArray p;
    auto app16 = [&](uint16_t v) {
        p.append(static_cast<char>(v >> 8));
        p.append(static_cast<char>(v & 0xFF));
    };
    app16(cfg.period_us);
    app16(cfg.pulse_min_us);
    app16(cfg.pulse_max_us);
    app16(cfg.pulse_neutral_us);
    return encode(Uner::CMD_SG90_CFG, p);
}

// 0x66: IR_DEBOUNCE_TICKS (1 byte)
QByteArray UnerProtocol::cmdIrDebounce(uint8_t debounce)
{
    QByteArray p; p.append(static_cast<char>(debounce));
    return encode(Uner::CMD_IR_DEBOUNCE, p);
}

// 0x67: retractTimer, SG90_ANGLE_DETECT, SG90_ANGLE_REPOSE, hcsrTimer,
//       aliveTimer, configCajas[0], configCajas[1], configCajas[2]
QByteArray UnerProtocol::cmdTimersCfg(const Uner::TimersCfg &cfg)
{
    QByteArray p;
    p.append(static_cast<char>(cfg.retract_timer));
    p.append(static_cast<char>(cfg.sg90_angle_detect));
    p.append(static_cast<char>(cfg.sg90_angle_repose));
    p.append(static_cast<char>(cfg.hcsr_timer));
    p.append(static_cast<char>(cfg.alive_timer));
    for (int i = 0; i < 3; i++) p.append(static_cast<char>(cfg.config_cajas[i]));
    return encode(Uner::CMD_TIMERS_CFG, p);
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
