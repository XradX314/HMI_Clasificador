# Modo Ciego — Design Spec
**Date:** 2026-05-16
**Status:** Approved

## Overview

Add a "Modo Ciego" toggle button to the control panel. When activated, it sends command `CMD_BLIND (0x63)` to the microcontroller via the UNER protocol, with the last measured belt velocity as payload. If no velocity has been measured in the current session, the system first prompts the user to perform a velocity measurement via the existing `VelocidadDialog` before activating.

## Command Protocol

- **Command:** `CMD_BLIND = 0x63` (next free after `CMD_MEDIR_VEL = 0x62`)
- **Direction:** PC → MCU
- **Payload:** 1 byte — belt velocity in cm/s (`uint8_t`)
- **Frame structure:** `UNER\x03:0x63 <vel_cm_s> <checksum>`
- **Firmware behavior:** toggle — each `0x63` alternates between blind mode and normal mode
- **No response expected** from MCU (fire & forget, same pattern as `CMD_STOP` and `CMD_RESET`)

## Architecture

### UnerProtocol (comunicacion/UnerProtocol.h/.cpp)

- Add constant: `constexpr uint8_t CMD_BLIND = 0x63;` in the `Uner` namespace
- Add static method: `static QByteArray cmdBlind(uint8_t velCmS);`
  - Encodes payload `{velCmS}` with `encode(CMD_BLIND, payload)`

### SerialManager (comunicacion/SerialManager.h/.cpp)

- Add public method: `void sendBlindMode(uint8_t velCmS);`
  - Validates connection is open before sending
  - Calls `UnerProtocol::cmdBlind(velCmS)` and writes to serial port

### MainWindow (MainWindow.h/.cpp)

**New state variables:**
- `uint8_t m_ultimaVelocidad {0}` — last measured velocity; 0 means not yet measured
- `bool m_pendingBlindMode {false}` — true when blind mode activation is waiting for a velocity measurement
- `QPushButton *m_btnBlindMode {nullptr}` — reference to the toggle button

**New slot:**
- `void onBlindModeToggled(bool checked);`

**Modified methods:**
- `buildPanelControl()` — add checkable `m_btnBlindMode` button after Reset
- `setRunningState(bool running)` — disable `m_btnBlindMode` while system is running
- `onVelocidadMedida(uint8_t vel)` — store `m_ultimaVelocidad = vel`; if `m_pendingBlindMode`, clear flag, call `sendBlindMode(vel)`, and set button checked

## Activation Flow

```
User clicks "Modo Ciego" (activating):
  ├─ m_ultimaVelocidad > 0?
  │     ├─ YES → m_serial->sendBlindMode(m_ultimaVelocidad)
  │     │         button stays checked
  │     └─ NO  → m_pendingBlindMode = true
  │               button unchecked (not active yet)
  │               open VelocidadDialog
  │               [user measures velocity]
  │               onVelocidadMedida(vel) fires:
  │                 m_ultimaVelocidad = vel
  │                 m_pendingBlindMode = false
  │                 m_serial->sendBlindMode(vel)
  │                 m_btnBlindMode->setChecked(true)
  └─ (deactivating, checked → unchecked)
        → m_serial->sendBlindMode(m_ultimaVelocidad)
          (firmware toggles back to normal)
```

## UI

- **Button label:** `Modo Ciego`
- **Position:** control panel, after the Reset button
- **Type:** `QPushButton`, checkable (`setCheckable(true)`)
- **Enabled:** only when disconnected or connected-and-stopped (disabled during `m_running == true`)
- **Styling:** inherits existing theme (dark/light mode) via `setDarkMode()`; checked state uses Qt's built-in checkable button visual

## Constraints

- `m_ultimaVelocidad` persists for the lifetime of the session; it is not reset on Stop/Reset
- `m_pendingBlindMode` is cleared on `onVelocidadMedida` (success) or if the user closes `VelocidadDialog` without measuring (connect to dialog's `rejected`/`closed` signal to reset the flag and uncheck the button)
- The blind mode button must be disabled while the belt is running, consistent with the output configuration panel behavior
- The heartbeat mechanism is unaffected; `CMD_ALIVE` handling continues normally in blind mode
- On disconnect or connection loss, `m_btnBlindMode` must be unchecked and `m_pendingBlindMode` reset to `false` — the MCU state resets on its end, so the HMI must mirror that
