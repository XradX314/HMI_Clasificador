# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Qt6 C++ desktop HMI (Human Machine Interface) for a conveyor-belt package classifier. The PC communicates with a microcontroller (MCU) over serial (UART) using a custom binary protocol called **UNER protocol**.

## Build

This is a qmake project (Qt 6.11, MinGW 64-bit on Windows). Open `HMI_Clasificador.pro` in Qt Creator and build from there, or from the command line:

```
qmake HMI_Clasificador.pro
mingw32-make
```

Build artifacts land in `build/Desktop_Qt_6_11_0_MinGW_64_bit-Debug/`. There are no tests.

Qt modules required: `core gui widgets serialport` (already declared in `.pro`).

## Architecture

Three layers communicating via Qt signals/slots:

```
MainWindow  ←→  SerialManager  ←→  UnerProtocol  ←→  QSerialPort
                      ↕
              ConfigDialog / VelocidadDialog / LedAliveWidget
```

### `comunicacion/UnerProtocol`
- **Encoder**: static methods (`cmdStart`, `cmdStop`, `cmdReset`, `cmdVelocidad`, `cmdConfig`, `cmdMedir`, `cmdMedirVelocidad`, `cmdBlind`) produce framed `QByteArray` frames.
- **Decoder**: `feed(data)` drives a byte-level state machine. Frame format: `UNER` + `len` + `:` + `cmd` + `payload` + XOR-checksum. Emits `frameReceived(Frame)` on success.
- All protocol constants (`CMD_*`, `PARAM_*`) and structs (`Frame`, `ConfigUmbrales`) live in the `Uner` namespace.

### `comunicacion/SerialManager`
Owns `QSerialPort` + `UnerProtocol`. Manages connection, heartbeat watchdog, and frame dispatch.

Two **exclusive-mode** flags gate the dispatch loop:
- `m_esperandoMedir` — set by `sendMedir()`, waits for 0x61 response (3 s timeout). All other frames are silently dropped while active.
- `m_esperandoVel` — set by `sendMedirVelocidad()`, waits for 0x62 response (60 s timeout). Same drop behavior.

Timers: heartbeat 7 s (`m_heartbeatTimer`), medir 3 s (`m_medirTimer`), velocity 60 s (`m_velTimer`).

### `MainWindow`
Orchestrates everything. Key state:
- `m_ultimaVelocidad` — last measured belt velocity in cm/s; `0` means not yet measured this session.
- `m_pendingBlindMode` — set when user clicks Modo Ciego before any velocity measurement has been taken; triggers `VelocidadDialog` flow, then auto-sends CMD_BLIND on `velocidadMedida`.

UI is built entirely in code (no `.ui` files). `buildTabMonitor()` composes six panels. Dark/light themes are large QSS string literals at the top of `MainWindow.cpp`.

### `widgets/`
- **`ConfigDialog`** — floating dialog for threshold configuration (CMD_MEDIR 0x61 live-measurement per field, then CMD_CONFIG 0x60 on apply).
- **`VelocidadDialog`** — floating dialog for belt velocity measurement (CMD_MEDIR_VEL 0x62, 60 s countdown progress bar).
- **`LedAliveWidget`** — animated LED that flashes on each 0xF0 heartbeat.

Both dialogs are lazily constructed (created on first open, reused on subsequent opens). They receive measurement results and timeouts via direct method calls from `MainWindow`, not signals from `SerialManager`.

## Protocol Reference

| CMD  | Direction | Payload | Purpose |
|------|-----------|---------|---------|
| 0xF0 | Both      | 0x0D (ACK) | Heartbeat / ACK |
| 0x50 | PC→MCU    | 0x00, s0, s1, s2 (TipoCaja) | Start classifier |
| 0x51 | PC→MCU    | 0x0D | Stop |
| 0x52 | MCU→PC    | servoIdx, 0x00 | Arm actuated |
| 0x53 | PC→MCU    | 0x0D | Reset |
| 0x54 | PC→MCU    | velIdx×10 | Set belt speed (index 1–10) |
| 0x5E | MCU→PC    | pairs (outNum, state) | IR sensor update |
| 0x5F | MCU→PC    | height_cm | Box detected |
| 0x60 | PC→MCU    | piso, peq, med, gde, tol (cm) | Threshold config |
| 0x61 | Both      | (empty) PC→MCU trigger; cm MCU→PC | Measure box height |
| 0x62 | Both      | ancho_cm PC→MCU; vel_cm_s MCU→PC | Measure belt velocity |
| 0x63 | PC→MCU    | vel_cm_s | Toggle blind mode |

`TipoCaja` values: `Ninguna=0`, `Pequeña=6`, `Mediana=8`, `Grande=10`.

## Adding New Commands

1. Add the `constexpr uint8_t CMD_*` to `comunicacion/UnerProtocol.h` (Uner namespace).
2. Add a static encoder method to `UnerProtocol` (`.h` declaration + `.cpp` implementation using `encode()`).
3. Add a `send*()` method to `SerialManager` that calls `sendRaw(UnerProtocol::cmd*(...))`.
4. Handle the incoming frame in `SerialManager::dispatchFrame()` and emit an appropriate signal.
5. Connect the signal in `MainWindow` constructor and implement the slot.
