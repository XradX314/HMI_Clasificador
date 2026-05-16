# Modo Ciego — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Agregar un botón toggle "Modo Ciego" al panel de control que envíe el comando 0x63 (CMD_BLIND) vía protocolo UNER con la velocidad de cinta como payload; si no hay velocidad medida, abre VelocidadDialog primero.

**Architecture:** Se agrega `CMD_BLIND = 0x63` al namespace Uner y su encoder estático en UnerProtocol. SerialManager expone `sendBlindMode(uint8_t)`. MainWindow maneja el flujo con dos variables de estado (`m_ultimaVelocidad`, `m_pendingBlindMode`) y un botón checkable que integra con el VelocidadDialog existente.

**Tech Stack:** Qt 6.11.0, C++17, QtSerialPort, MinGW 64-bit

---

## Archivos afectados

| Acción | Archivo |
|--------|---------|
| Modificar | `comunicacion/UnerProtocol.h` |
| Modificar | `comunicacion/UnerProtocol.cpp` |
| Modificar | `comunicacion/SerialManager.h` |
| Modificar | `comunicacion/SerialManager.cpp` |
| Modificar | `MainWindow.h` |
| Modificar | `MainWindow.cpp` |

---

## Task 1: Agregar CMD_BLIND a UnerProtocol

**Files:**
- Modify: `comunicacion/UnerProtocol.h`
- Modify: `comunicacion/UnerProtocol.cpp`

- [ ] **Step 1: Agregar la constante CMD_BLIND al namespace Uner**

En `comunicacion/UnerProtocol.h`, reemplazar la línea 28:
```cpp
constexpr uint8_t CMD_MEDIR_VEL   = 0x62;   // PC→MCU: uint8_t ancho_cm | MCU→PC: uint8_t vel_cm_s
```
Con:
```cpp
constexpr uint8_t CMD_MEDIR_VEL   = 0x62;   // PC→MCU: uint8_t ancho_cm | MCU→PC: uint8_t vel_cm_s
constexpr uint8_t CMD_BLIND       = 0x63;   // PC→MCU: uint8_t vel_cm_s (activa/desactiva modo ciego)
```

- [ ] **Step 2: Actualizar el comentario de cabecera del archivo**

En `comunicacion/UnerProtocol.h`, reemplazar la línea del comentario del protocolo:
```cpp
//  PC  → MCU: 0xF0 ACK | 0x50 Start | 0x51 Stop | 0x52 Brazo
//             0x53 Reset | 0x54 Velocidad | 0x60 Config umbrales
```
Con:
```cpp
//  PC  → MCU: 0xF0 ACK | 0x50 Start | 0x51 Stop | 0x52 Brazo
//             0x53 Reset | 0x54 Velocidad | 0x60 Config umbrales
//             0x63 Modo ciego (toggle, payload: vel_cm_s)
```

- [ ] **Step 3: Declarar cmdBlind() en la clase UnerProtocol**

En `comunicacion/UnerProtocol.h`, después de la línea con `cmdMedirVelocidad`:
```cpp
static QByteArray cmdMedirVelocidad(uint8_t anchoCm);  // 0x62
```
Agregar:
```cpp
static QByteArray cmdBlind(uint8_t velCmS);            // 0x63
```

- [ ] **Step 4: Implementar cmdBlind() en UnerProtocol.cpp**

En `comunicacion/UnerProtocol.cpp`, después de la función `cmdMedirVelocidad()` (línea 93), agregar:
```cpp
QByteArray UnerProtocol::cmdBlind(uint8_t velCmS)
{
    QByteArray p;
    p.append(static_cast<char>(velCmS));
    return encode(Uner::CMD_BLIND, p);
}
```

- [ ] **Step 5: Verificar que el proyecto compila sin errores**

Compilar con Qt Creator o desde terminal:
```
cd "C:\Users\gonza\Downloads\HMI_Clasificador (7)\HMI_Clasificador"
qmake && mingw32-make -j4
```
Resultado esperado: compilación exitosa, sin warnings nuevos.

- [ ] **Step 6: Commit**

```
git add comunicacion/UnerProtocol.h comunicacion/UnerProtocol.cpp
git commit -m "feat: add CMD_BLIND (0x63) encoder to UnerProtocol"
```

---

## Task 2: Exponer sendBlindMode en SerialManager

**Files:**
- Modify: `comunicacion/SerialManager.h`
- Modify: `comunicacion/SerialManager.cpp`

- [ ] **Step 1: Declarar sendBlindMode() en SerialManager.h**

En `comunicacion/SerialManager.h`, después de la línea con `sendMedirVelocidad`:
```cpp
void sendMedirVelocidad(uint8_t anchoCm);   // 0x62, espera hasta 60 s
```
Agregar:
```cpp
void sendBlindMode(uint8_t velCmS);          // 0x63, fire & forget
```

- [ ] **Step 2: Implementar sendBlindMode() en SerialManager.cpp**

En `comunicacion/SerialManager.cpp`, después de la función `sendMedirVelocidad()` (alrededor de línea 134), agregar:
```cpp
void SerialManager::sendBlindMode(uint8_t velCmS)
{
    sendRaw(UnerProtocol::cmdBlind(velCmS));
}
```

- [ ] **Step 3: Verificar que el proyecto compila sin errores**

```
qmake && mingw32-make -j4
```
Resultado esperado: compilación exitosa.

- [ ] **Step 4: Commit**

```
git add comunicacion/SerialManager.h comunicacion/SerialManager.cpp
git commit -m "feat: add sendBlindMode() to SerialManager"
```

---

## Task 3: Agregar estado y slot a MainWindow.h

**Files:**
- Modify: `MainWindow.h`

- [ ] **Step 1: Agregar el slot onBlindModeToggled**

En `MainWindow.h`, en la sección `private slots:`, después de la línea `void onVelocidadTimeout();` (línea 55), agregar:
```cpp
void onBlindModeToggled(bool checked);
```

- [ ] **Step 2: Agregar las variables de estado**

En `MainWindow.h`, en la sección `// Estado`, después de la línea con `m_pendingMedirField` (línea 76), agregar:
```cpp
uint8_t m_ultimaVelocidad {0};         // 0 = no medida aún en esta sesión
bool    m_pendingBlindMode {false};    // esperando medición de velocidad para activar
```

- [ ] **Step 3: Agregar el puntero al botón**

En `MainWindow.h`, en la sección `// Tab: Monitor`, después de la línea con `m_btnReset` (línea 91), agregar:
```cpp
QPushButton *m_btnBlindMode {nullptr};
```

- [ ] **Step 4: Verificar que el proyecto compila sin errores**

```
qmake && mingw32-make -j4
```
Resultado esperado: compilación exitosa.

---

## Task 4: Agregar botón y estilos en MainWindow.cpp

**Files:**
- Modify: `MainWindow.cpp`

- [ ] **Step 1: Agregar estilos del botón en STYLE_LIGHT**

En `MainWindow.cpp`, dentro del string `STYLE_LIGHT`, después de las líneas de `#btnConectar:hover` (alrededor de línea 57), agregar:
```css
QPushButton#btnBlindMode { background: #F5F3FF; border-color: #C4B5FD; color: #5B21B6; }
QPushButton#btnBlindMode:hover { background: #EDE9FE; }
QPushButton#btnBlindMode:checked { background: #7C3AED; border-color: #6D28D9; color: #FFFFFF; }
QPushButton#btnBlindMode:checked:hover { background: #6D28D9; }
QPushButton#btnBlindMode:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }
```

- [ ] **Step 2: Agregar estilos del botón en STYLE_DARK**

En `MainWindow.cpp`, dentro del string `STYLE_DARK`, después de las líneas de `#btnConectar:hover` (alrededor de línea 106), agregar:
```css
QPushButton#btnBlindMode { background: #2E1065; border-color: #5B21B6; color: #C4B5FD; }
QPushButton#btnBlindMode:hover { background: #3B0764; }
QPushButton#btnBlindMode:checked { background: #7C3AED; border-color: #6D28D9; color: #FFFFFF; }
QPushButton#btnBlindMode:checked:hover { background: #6D28D9; }
QPushButton#btnBlindMode:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
```

- [ ] **Step 3: Crear el botón en buildPanelControl()**

En `MainWindow.cpp`, en `buildPanelControl()`, reemplazar (líneas 304-309):
```cpp
    m_btnReset = new QPushButton("↺  Reset");
    m_btnReset->setObjectName("btnReset");
    connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    vl->addWidget(m_btnReset);

    vl->addSpacing(6);
```
Con:
```cpp
    m_btnReset = new QPushButton("↺  Reset");
    m_btnReset->setObjectName("btnReset");
    connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    vl->addWidget(m_btnReset);

    m_btnBlindMode = new QPushButton("Modo Ciego");
    m_btnBlindMode->setObjectName("btnBlindMode");
    m_btnBlindMode->setCheckable(true);
    m_btnBlindMode->setEnabled(false);
    connect(m_btnBlindMode, &QPushButton::toggled, this, &MainWindow::onBlindModeToggled);
    vl->addWidget(m_btnBlindMode);

    vl->addSpacing(6);
```

- [ ] **Step 4: Habilitar/deshabilitar en setConnectedState()**

En `MainWindow.cpp`, en `setConnectedState()` (alrededor de línea 762-769), reemplazar:
```cpp
void MainWindow::setConnectedState(bool connected)
{
    m_btnStart->setEnabled(connected && !m_running);
    m_btnStop->setEnabled(false);
    m_btnReset->setEnabled(connected);
    m_spinVel->setEnabled(connected);
    checkConfigLock();
}
```
Con:
```cpp
void MainWindow::setConnectedState(bool connected)
{
    m_btnStart->setEnabled(connected && !m_running);
    m_btnStop->setEnabled(false);
    m_btnReset->setEnabled(connected);
    m_spinVel->setEnabled(connected);
    m_btnBlindMode->setEnabled(connected && !m_running);
    checkConfigLock();
}
```

- [ ] **Step 5: Deshabilitar durante running en setRunningState()**

En `MainWindow.cpp`, en `setRunningState()` (alrededor de línea 771-777), reemplazar:
```cpp
void MainWindow::setRunningState(bool running)
{
    m_running = running;
    m_btnStart->setEnabled(!running && m_serial->isOpen());
    m_btnStop->setEnabled(running);
    checkConfigLock();
}
```
Con:
```cpp
void MainWindow::setRunningState(bool running)
{
    m_running = running;
    m_btnStart->setEnabled(!running && m_serial->isOpen());
    m_btnStop->setEnabled(running);
    m_btnBlindMode->setEnabled(!running && m_serial->isOpen());
    checkConfigLock();
}
```

- [ ] **Step 6: Verificar que el proyecto compila y el botón aparece en la UI**

```
qmake && mingw32-make -j4
```
Lanzar la aplicación y verificar visualmente:
- El botón "Modo Ciego" aparece debajo de Reset en el panel de control
- El botón está deshabilitado cuando no hay conexión
- El botón cambia de color al hacer click (estado checked)

- [ ] **Step 7: Commit**

```
git add MainWindow.h MainWindow.cpp
git commit -m "feat: add Modo Ciego toggle button to control panel"
```

---

## Task 5: Implementar la lógica de activación

**Files:**
- Modify: `MainWindow.cpp`

- [ ] **Step 1: Actualizar onVelocidadMedida() para almacenar la velocidad**

En `MainWindow.cpp`, reemplazar la función `onVelocidadMedida()` completa (líneas 681-685):
```cpp
void MainWindow::onVelocidadMedida(uint8_t velCmS)
{
    if (m_velocidadDialog) m_velocidadDialog->velocidadRecibida(velCmS);
    statusMsg(QString("Velocidad de cinta: %1 cm/s").arg(velCmS));
}
```
Con:
```cpp
void MainWindow::onVelocidadMedida(uint8_t velCmS)
{
    m_ultimaVelocidad = velCmS;
    if (m_velocidadDialog) m_velocidadDialog->velocidadRecibida(velCmS);
    statusMsg(QString("Velocidad de cinta: %1 cm/s").arg(velCmS));

    if (m_pendingBlindMode) {
        m_pendingBlindMode = false;
        m_serial->sendBlindMode(velCmS);
        m_btnBlindMode->setChecked(true);
        statusMsg(QString("Modo ciego activado — velocidad: %1 cm/s").arg(velCmS));
    }
}
```

- [ ] **Step 2: Implementar onBlindModeToggled()**

En `MainWindow.cpp`, después de `onVelocidadTimeout()` (alrededor de línea 691), agregar:
```cpp
void MainWindow::onBlindModeToggled(bool checked)
{
    if (!m_serial->isOpen()) {
        m_btnBlindMode->setChecked(false);
        return;
    }

    if (!checked) {
        // Desactivando modo ciego — el firmware alterna al recibir 0x63
        m_serial->sendBlindMode(m_ultimaVelocidad);
        statusMsg("Modo ciego desactivado.");
        return;
    }

    // Activando
    if (m_ultimaVelocidad > 0) {
        m_serial->sendBlindMode(m_ultimaVelocidad);
        statusMsg(QString("Modo ciego activado — velocidad: %1 cm/s").arg(m_ultimaVelocidad));
    } else {
        // Sin velocidad medida: abrir diálogo y esperar
        m_pendingBlindMode = true;
        m_btnBlindMode->setChecked(false);
        onOpenVelocidad();
        // Si el usuario cierra el diálogo sin medir, cancelar la activación pendiente
        connect(m_velocidadDialog, &QDialog::finished, this, [this](int) {
            if (m_pendingBlindMode) {
                m_pendingBlindMode = false;
                m_btnBlindMode->setChecked(false);
            }
        }, Qt::SingleShotConnection);
        statusMsg("Medí la velocidad de cinta para activar el modo ciego.", 5000);
    }
}
```

- [ ] **Step 3: Resetear estado en onDisconnected()**

En `MainWindow.cpp`, en `onDisconnected()` (líneas 514-523), reemplazar:
```cpp
void MainWindow::onDisconnected()
{
    setConnectedState(false);
    setRunningState(false);
    m_ledAlive->reset();
    m_lblConexion->setText("Desconectado");
    m_lblConexion->setStyleSheet("font-size: 12px; color: #6B7280;");
    m_btnConectar->setText("Conectar");
    statusMsg("Desconectado.");
}
```
Con:
```cpp
void MainWindow::onDisconnected()
{
    m_pendingBlindMode = false;
    m_btnBlindMode->setChecked(false);
    setConnectedState(false);
    setRunningState(false);
    m_ledAlive->reset();
    m_lblConexion->setText("Desconectado");
    m_lblConexion->setStyleSheet("font-size: 12px; color: #6B7280;");
    m_btnConectar->setText("Conectar");
    statusMsg("Desconectado.");
}
```

- [ ] **Step 4: Verificar que el proyecto compila sin errores**

```
qmake && mingw32-make -j4
```
Resultado esperado: compilación exitosa, sin warnings nuevos.

- [ ] **Step 5: Prueba del flujo con velocidad ya medida**

1. Conectar a un puerto (real o loopback)
2. Abrir VelocidadDialog desde la toolbar (⚡), medir velocidad (o simularla)
3. Cerrar VelocidadDialog
4. Hacer click en "Modo Ciego"
5. Verificar en logs de debug (`QDebug`): debe aparecer `[SerialManager] TX:` con el frame `55 4e 45 52 03 3a 63 <vel> <cks>`
6. El botón debe quedar visualmente activo (checked, fondo violeta)
7. Hacer click nuevamente → botón vuelve a inactivo, nuevo TX en los logs

- [ ] **Step 6: Prueba del flujo sin velocidad medida**

1. Reiniciar la aplicación (para resetear `m_ultimaVelocidad = 0`)
2. Conectar a un puerto
3. Hacer click en "Modo Ciego" sin haber medido velocidad antes
4. Verificar: VelocidadDialog se abre automáticamente
5. Cerrar VelocidadDialog sin medir → botón "Modo Ciego" queda sin activar
6. Volver a click en "Modo Ciego" → VelocidadDialog se abre de nuevo
7. Medir velocidad → VelocidadDialog cierra o queda abierto, botón "Modo Ciego" se activa automáticamente, TX aparece en logs

- [ ] **Step 7: Prueba de desconexión con modo activo**

1. Activar Modo Ciego
2. Desconectar el puerto
3. Verificar: botón queda sin activar (unchecked), `m_pendingBlindMode = false`

- [ ] **Step 8: Commit final**

```
git add MainWindow.cpp
git commit -m "feat: implement Modo Ciego activation flow with velocity prerequisite"
```
