# Design: Velocidad continua + Visualizador de cinta

**Date:** 2026-05-19  
**Branch:** `feature/velocidad-continua-visualizador` (rama nueva desde master)

---

## Contexto

El sistema actual mide la velocidad de la cinta una sola vez: el PC envía CMD 0x62 con `anchoCaja`, el MCU activa `midiendo_vel` y mide en el siguiente paso por IR0. El resultado queda fijo hasta la próxima sesión.

Este spec agrega:
1. **Velocidad continua**: el MCU recalcula la velocidad en *cada* paso de caja por IR0 y la reporta al PC via CMD 0x62 de respuesta.
2. **Visualizador**: ventana emergente no bloqueante con animación 2D lateral de las cajas en movimiento sobre la cinta.

---

## Feature 1 — Velocidad continua

### Firmware (`Actividad4/main.c`)

**`on_s0_detected()`** — quitar el gate `if (midiendo_vel)`:
```c
// Antes (solo cuando midiendo_vel):
if (midiendo_vel) { t_detect = g_now_us; }

// Después (siempre):
t_detect = g_now_us;
```

**`on_s0_released()`** — quitar el gate, siempre calcular y reportar:
```c
// Reemplazar bloque if (midiendo_vel) { ... } por:
if (t_detect > 0 && anchoCaja > 0) {
    uint32_t dt_us = g_now_us - t_detect;
    if (dt_us > 0) {
        uint16_t vel = (uint16_t)(((uint32_t)anchoCaja * 1000000UL) / dt_us);
        if (vel > 255) vel = 255;
        vel_cinta_cms = (float)vel;
        vel_medida    = 1;
        uint8_t vbyte = (uint8_t)vel;
        Encode(0x62, &vbyte, 1);   // MCU → PC: velocidad medida
    }
}
```

**`CmdParser case 0x62`** — simplificar a solo actualizar `anchoCaja`:
```c
case 0x62:
    anchoCaja = params[0];
    // No se llama a App_IniciarVelocidad; la medición es automática ahora.
    break;
```

**`App_IniciarVelocidad()`** y variable `midiendo_vel`** — eliminar ambas (ya sin uso).

**`CmdParser case 0x60`** — eliminar asignación `midiendo_vel = 1`.

**`clasificador.h`** — eliminar `extern uint8_t midiendo_vel;`.

### HMI — `SerialManager`

Nueva señal en `SerialManager.h`:
```cpp
void velocidadCintaActualizada(uint8_t vel_cm_s);
```

En `dispatchFrame()` (`SerialManager.cpp`), nuevo case:
```cpp
case Uner::CMD_ANCHO_CAJA:   // 0x62 bidireccional: MCU→PC = velocidad medida
    if (!frame.payload.isEmpty())
        emit velocidadCintaActualizada(static_cast<uint8_t>(frame.payload.at(0)));
    break;
```

### HMI — `MainWindow`

Nuevo miembro en `MainWindow.h`:
```cpp
uint8_t      m_velCintaCms   {0};      ///< Última velocidad medida (cm/s).
QLabel      *m_lblVelCinta   {nullptr};///< Label de velocidad en panel de control.
```

En `buildPanelControl()` (`MainWindow.cpp`): añadir label debajo del SpinBox de velocidad:
```cpp
m_lblVelCinta = new QLabel("Vel. cinta: -- cm/s");
m_lblVelCinta->setObjectName("lblVelCinta");
vl->addWidget(m_lblVelCinta);
```

Slot nuevo `onVelocidadCintaActualizada(uint8_t vel)`:
```cpp
void MainWindow::onVelocidadCintaActualizada(uint8_t vel) {
    m_velCintaCms = vel;
    m_lblVelCinta->setText(QString("Vel. cinta: %1 cm/s").arg(vel));
    if (m_visualizadorDialog)
        m_visualizadorDialog->canvas()->onVelocidadActualizada(vel);
}
```

Conectar en constructor:
```cpp
connect(m_serial, &SerialManager::velocidadCintaActualizada,
        this,     &MainWindow::onVelocidadCintaActualizada);
```

---

## Feature 2 — Visualizador (`CintaVisualizador`)

### Archivos nuevos

- `widgets/CintaVisualizador.h`
- `widgets/CintaVisualizador.cpp`

### Estructuras

```cpp
struct CajaEnCinta {
    float   posX;       // cm desde IR0
    uint8_t tipo;       // 6=P, 8=M, 10=G, 0=desconocido
    uint8_t alturaCm;
};

struct BrazoAnim {
    bool activo   {false};
    int  msQuedan {0};  // duración de la animación
};
```

### `CintaCanvas : QWidget`

**Estado interno:**
```cpp
QList<CajaEnCinta> m_cajas;
BrazoAnim          m_brazos[3];
float              m_velCmS   {0.0f};
uint8_t            m_dist[3]  {30, 60, 90};  // cm desde IR0 a cada servo/IR
uint8_t            m_anchoCaja{10};
QElapsedTimer      m_elapsed;
QTimer             m_timer;                  // 30 fps
```

**`paintEvent()`** — en coordenadas de píxeles escaladas desde cm:

| Elemento | Visual |
|---|---|
| Cinta | `QRect` gris, dos círculos (rodillos) en extremos |
| IR0 | Línea vertical + etiqueta "IR0" en posX=0 |
| IR1/2/3 | Línea vertical + etiqueta en posX=dist[0/1/2] |
| Servo0/1/2 | Pequeño triángulo o rect debajo de la cinta, mismo X que IRx correspondiente |
| Caja | `QRect` coloreado (verde=P, azul=M, naranja=G), altura ∝ alturaCm, letra centrada |
| Brazo activo | Rect que emerge hacia arriba desde la cinta durante `msQuedan > 0` |

**Factor de escala:**
```cpp
float totalCm  = m_dist[2] + m_anchoCaja * 3;  // margen a la derecha
float pixPerCm = (width() - 40) / totalCm;      // 20px padding cada lado
```

**Tick del timer (30 fps):**
```cpp
float dt = m_elapsed.restart() / 1000.0f;
for (auto &c : m_cajas) c.posX += m_velCmS * dt;
m_cajas.removeIf([&](const CajaEnCinta &c){ return c.posX > m_dist[2] + 30; });
for (auto &b : m_brazos) {
    if (b.activo) { b.msQuedan -= static_cast<int>(dt * 1000); if (b.msQuedan <= 0) b.activo = false; }
}
update();
```

**Slots públicos:**
```cpp
void onCajaMedida(uint8_t alturaCm);          // nueva caja en posX=0
void onSensorIr(uint8_t outNum, bool activo); // outNum 0/1/2 → snap a dist[outNum]; outNum 3 = IR0, ignorar (cubierto por cajaMedida)
void onBrazoActuado(uint8_t servoIdx);        // activo=true, msQuedan=400; brazo en dist[servoIdx]
void onVelocidadActualizada(uint8_t vel);
void setDistancias(uint8_t d0, uint8_t d1, uint8_t d2);
void setAnchoCaja(uint8_t ancho);
```

**Mapping de outNum (firmware → visualizador):**
| Firmware payload outNum | Sensor físico | Posición en visualizador |
|---|---|---|
| 3 | IR0 (entrada, HC-SR04) | posX = 0 — cubierto por `cajaMedida` |
| 0 | IR1 (salida 0) | posX = dist[0] |
| 1 | IR2 (salida 1) | posX = dist[1] |
| 2 | IR3 (salida 2) | posX = dist[2] |

### `CintaVisualizador : QDialog`

```cpp
class CintaVisualizador : public QDialog {
    Q_OBJECT
public:
    explicit CintaVisualizador(QWidget *parent = nullptr);
    CintaCanvas *canvas() const { return m_canvas; }
    void setDarkMode(bool dark);
private:
    CintaCanvas *m_canvas {nullptr};
};
```

- No modal (`setModal(false)`)
- Tamaño mínimo: 700 × 220 px
- Título: "Visualizador de cinta"
- `CintaCanvas` ocupa todo el espacio del diálogo (layout simple)

### `MainWindow` — integración

Nuevos miembros en `MainWindow.h`:
```cpp
CintaVisualizador *m_visualizadorDialog {nullptr};
uint8_t            m_anchoCaja          {10};
```

Botón en toolbar (`buildToolBar()`):
```cpp
auto *btnVis = new QPushButton("Visualizar");
btnVis->setToolTip("Visualizador en tiempo real de la cinta");
connect(btnVis, &QPushButton::clicked, this, &MainWindow::onOpenVisualizador);
tb->addWidget(btnVis);
```

Slot `onOpenVisualizador()`:
```cpp
void MainWindow::onOpenVisualizador() {
    if (!m_visualizadorDialog) {
        m_visualizadorDialog = new CintaVisualizador(this);
        m_visualizadorDialog->setDarkMode(m_darkMode);
        auto *cv = m_visualizadorDialog->canvas();
        cv->setDistancias(m_ciegoCfg.dist_s0[0], m_ciegoCfg.dist_s0[1], m_ciegoCfg.dist_s0[2]);
        cv->setAnchoCaja(m_anchoCaja);
        cv->onVelocidadActualizada(m_velCintaCms);
        connect(m_serial, &SerialManager::cajaMedida,
                cv,        &CintaCanvas::onCajaMedida);
        connect(m_serial, &SerialManager::sensorIrActualizado,
                cv,        &CintaCanvas::onSensorIr);
        connect(m_serial, &SerialManager::brazoActuado,
                cv,        &CintaCanvas::onBrazoActuado);
    }
    m_visualizadorDialog->show();
    m_visualizadorDialog->raise();
    m_visualizadorDialog->activateWindow();
}
```

Actualizar `m_anchoCaja` en `onAnchoCajaRequested()`:
```cpp
m_anchoCaja = anchoCm;
if (m_visualizadorDialog) m_visualizadorDialog->canvas()->setAnchoCaja(anchoCm);
```

Aplicar tema en `onThemeToggled()`:
```cpp
if (m_visualizadorDialog) m_visualizadorDialog->setDarkMode(dark);
```

---

## Protocolo actualizado (CMD 0x62)

| CMD | Dirección | Payload | Propósito |
|-----|-----------|---------|-----------|
| 0x62 | PC → MCU | `anchoCaja` (1 byte, cm) | Informar ancho de referencia |
| 0x62 | MCU → PC | `vel_cm_s` (1 byte, cm/s) | Reportar velocidad medida (tras cada caja en IR0) |

---

## Rama de trabajo

```bash
git checkout -b feature/velocidad-continua-visualizador
```

Todos los commits van en esta rama. Se abre PR a master al finalizar.

---

## Archivos afectados

| Archivo | Cambio |
|---|---|
| `Actividad4/main.c` | Velocidad siempre-on, encode 0x62, eliminar `midiendo_vel` |
| `Actividad4/clasificador.h` | Eliminar `extern uint8_t midiendo_vel` |
| `comunicacion/SerialManager.h` | Nueva señal `velocidadCintaActualizada` |
| `comunicacion/SerialManager.cpp` | Dispatch de CMD 0x62 entrante |
| `MainWindow.h` | Nuevos miembros: `m_velCintaCms`, `m_lblVelCinta`, `m_visualizadorDialog`, `m_anchoCaja` |
| `MainWindow.cpp` | Label vel, slot `onVelocidadCintaActualizada`, slot `onOpenVisualizador`, botón toolbar |
| `widgets/CintaVisualizador.h` | Nuevo archivo |
| `widgets/CintaVisualizador.cpp` | Nuevo archivo |
| `HMI_Clasificador.pro` | Añadir `CintaVisualizador` a HEADERS y SOURCES |
