# Velocidad Continua + Visualizador de Cinta — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** El MCU recalcula velocidad en cada paso por IR0 y la reporta al HMI via CMD 0x62, y el HMI muestra una animación 2D de cajas sobre la cinta en tiempo real.

**Architecture:** Firmware siempre mide en `on_s0_released()` y codifica respuesta 0x62. SerialManager despacha el frame entrante 0x62 como nueva señal. `CintaVisualizador` (QDialog no modal con `CintaCanvas : QWidget`) actualiza posiciones a 30 fps con QPainter.

**Tech Stack:** Qt 6.11, C++17, QtSerialPort, MinGW 64-bit, AVR-GCC (ATmega328P firmware)

**Branch:** `feature/velocidad-continua-visualizador`

---

## File Map

| Archivo | Acción | Responsabilidad |
|---|---|---|
| `Actividad4/main.c` | Modificar | Quitar `midiendo_vel`, medir siempre en IR0 |
| `Actividad4/clasificador.h` | Modificar | Eliminar `extern uint8_t midiendo_vel` |
| `Actividad4/clasificador.c` | Modificar | Simplificar case 0x60 y case 0x62 |
| `comunicacion/SerialManager.h` | Modificar | Nueva señal `velocidadCintaActualizada` |
| `comunicacion/SerialManager.cpp` | Modificar | Despachar CMD 0x62 entrante |
| `MainWindow.h` | Modificar | Nuevos miembros y slots |
| `MainWindow.cpp` | Modificar | Label vel, botón toolbar, slots, conexiones, tema |
| `widgets/CintaVisualizador.h` | Crear | Declaraciones de CintaCanvas y CintaVisualizador |
| `widgets/CintaVisualizador.cpp` | Crear | Implementación completa |
| `HMI_Clasificador.pro` | Modificar | Registrar CintaVisualizador |

---

## Task 1: Crear rama de trabajo

**Files:**
- (ninguno — solo git)

- [ ] **Step 1: Crear y checkoutear la rama**

```bash
git checkout -b feature/velocidad-continua-visualizador
```

Expected: `Switched to a new branch 'feature/velocidad-continua-visualizador'`

---

## Task 2: Firmware — velocidad continua

**Files:**
- Modify: `Actividad-N-4-Microcontroladores-2026-main/Actividad4/main.c`
- Modify: `Actividad-N-4-Microcontroladores-2026-main/Actividad4/clasificador.h`
- Modify: `Actividad-N-4-Microcontroladores-2026-main/Actividad4/clasificador.c`

- [ ] **Step 1: Eliminar `midiendo_vel` de `clasificador.h`**

En `clasificador.h` línea 51, eliminar esta línea completa:
```c
extern uint8_t midiendo_vel;
```

- [ ] **Step 2: Simplificar case 0x60 en `clasificador.c`**

En `clasificador.c`, reemplazar el case 0x60 completo (líneas 267–283):
```c
		case 0x60:
			if (len >= 1) {
				modo_ciego = params[0];
			}
			if (len >= 2) medir_auto = params[1];
			if (len >= 5) {
				dist_s0_a_salida[0] = params[2];
				dist_s0_a_salida[1] = params[3];
				dist_s0_a_salida[2] = params[4];
			}
			if (modo_ciego == 1 && medir_auto == 1) {
				vel_medida = 0;
				midiendo_vel = 1;   
			} else {
				vel_medida = 1;     
			}
		break;
```

Por:
```c
		case 0x60:
			if (len >= 1) {
				modo_ciego = params[0];
			}
			if (len >= 2) medir_auto = params[1];
			if (len >= 5) {
				dist_s0_a_salida[0] = params[2];
				dist_s0_a_salida[1] = params[3];
				dist_s0_a_salida[2] = params[4];
			}
			vel_medida = 1;
		break;
```

- [ ] **Step 3: Simplificar case 0x62 en `clasificador.c`**

En `clasificador.c`, reemplazar el case 0x62 (líneas 287–290):
```c
		case 0x62:
			anchoCaja = params[0];
			if (_velocidad) _velocidad(params[0]);
		break;
```

Por:
```c
		case 0x62:
			anchoCaja = params[0];
		break;
```

- [ ] **Step 4: Eliminar `midiendo_vel` de `main.c`**

En `main.c` línea 14, eliminar esta línea:
```c
uint8_t midiendo_vel = 0;  
```

- [ ] **Step 5: Simplificar `App_IniciarVelocidad` en `main.c`**

En `main.c` líneas 343–346, reemplazar:
```c
void App_IniciarVelocidad(uint8_t ancho_cm) {
	anchoCaja = ancho_cm;
	midiendo_vel = 1;
}
```

Por:
```c
void App_IniciarVelocidad(uint8_t ancho_cm) {
	anchoCaja = ancho_cm;
}
```

- [ ] **Step 6: Hacer `t_detect` siempre activo en `on_s0_detected()`**

En `main.c`, reemplazar el bloque completo de `on_s0_detected()` (líneas 220–235):
```c
void on_s0_detected(void) {
	if (modo_ciego && vel_medida && vel_cinta_cms > 0) {
		for (uint8_t i = 0; i < 3; i++) {
			timer_ciego[i] = (uint16_t)((dist_s0_a_salida[i] / vel_cinta_cms) * 500.0f);
			timer_ciego_activo[i] = 1;
		}
	}
	if (midiendo_vel) {
		t_detect = g_now_us;
	}
	PORTD |= (1 << PORTD6);
	payload[0] = 0x03;
	payload[1] = 0x01;
	Encode(0x5E, payload, 2);
	App_TriggerHCSR04();
	//Encode(0x5F, payload, 1);
}
```

Por:
```c
void on_s0_detected(void) {
	if (modo_ciego && vel_medida && vel_cinta_cms > 0) {
		for (uint8_t i = 0; i < 3; i++) {
			timer_ciego[i] = (uint16_t)((dist_s0_a_salida[i] / vel_cinta_cms) * 500.0f);
			timer_ciego_activo[i] = 1;
		}
	}
	t_detect = g_now_us;
	PORTD |= (1 << PORTD6);
	payload[0] = 0x03;
	payload[1] = 0x01;
	Encode(0x5E, payload, 2);
	App_TriggerHCSR04();
}
```

- [ ] **Step 7: Hacer `on_s0_released()` siempre medir y reportar**

En `main.c`, reemplazar el bloque completo de `on_s0_released()` (líneas 238–255):
```c
void on_s0_released(void) {
	if (midiendo_vel) {
		uint32_t dt_us = g_now_us - t_detect;
		uint16_t vel = (uint16_t)(((uint32_t)anchoCaja * 1000000UL) / dt_us);
		
		if (vel > 255) vel = 255;
		
		vel_cinta_cms = (float)vel;

		vel_medida = 1;
		midiendo_vel = 0;
	}
	PORTD &= ~(1 << PORTD6);
	payload[0] = 0x03;
	payload[1] = 0x00;
	Encode(0x5E, payload, 2);
	//Encode(0x5F, payload, 1);
}
```

Por:
```c
void on_s0_released(void) {
	if (t_detect > 0 && anchoCaja > 0) {
		uint32_t dt_us = g_now_us - t_detect;
		if (dt_us > 0) {
			uint16_t vel = (uint16_t)(((uint32_t)anchoCaja * 1000000UL) / dt_us);
			if (vel > 255) vel = 255;
			vel_cinta_cms = (float)vel;
			vel_medida    = 1;
			uint8_t vbyte = (uint8_t)vel;
			Encode(0x62, &vbyte, 1);
		}
	}
	PORTD &= ~(1 << PORTD6);
	payload[0] = 0x03;
	payload[1] = 0x00;
	Encode(0x5E, payload, 2);
}
```

- [ ] **Step 8: Commit firmware**

```bash
git add Actividad-N-4-Microcontroladores-2026-main/Actividad4/main.c
git add Actividad-N-4-Microcontroladores-2026-main/Actividad4/clasificador.h
git add Actividad-N-4-Microcontroladores-2026-main/Actividad4/clasificador.c
git commit -m "feat(fw): continuous velocity measurement on every IR0 passage, report via 0x62"
```

---

## Task 3: SerialManager — señal `velocidadCintaActualizada`

**Files:**
- Modify: `comunicacion/SerialManager.h:196-204`
- Modify: `comunicacion/SerialManager.cpp:140-179`

- [ ] **Step 1: Agregar señal en `SerialManager.h`**

En `SerialManager.h`, después de la señal `brazoActuado` (línea 203), agregar:
```cpp
    /**
     * @brief Se emite cuando el MCU reporta la velocidad medida (0x62 MCU→PC).
     * @param vel_cm_s Velocidad de la cinta en cm/s.
     */
    void velocidadCintaActualizada(uint8_t vel_cm_s);
```

- [ ] **Step 2: Despachar CMD 0x62 entrante en `SerialManager.cpp`**

En `SerialManager.cpp`, dentro de `dispatchFrame()`, al final del switch (antes del cierre `}`), agregar:

```cpp
    // 0x62 MCU→PC: velocidad medida (el PC→MCU ya lo envía via sendAnchoCaja)
    case Uner::CMD_ANCHO_CAJA:
        if (!frame.payload.isEmpty()) {
            const uint8_t vel = static_cast<uint8_t>(frame.payload.at(0));
            emit velocidadCintaActualizada(vel);
            qDebug() << "[SerialManager] Vel. cinta:" << vel << "cm/s";
        }
        break;
```

- [ ] **Step 3: Compilar el proyecto en Qt Creator**

Abrir `HMI_Clasificador.pro` en Qt Creator → Build → Build Project.
Expected: 0 errores, 0 warnings nuevos sobre SerialManager.

- [ ] **Step 4: Commit SerialManager**

```bash
git add comunicacion/SerialManager.h comunicacion/SerialManager.cpp
git commit -m "feat(hmi): add velocidadCintaActualizada signal for 0x62 MCU->PC"
```

---

## Task 4: MainWindow.h — nuevos miembros y slots

**Files:**
- Modify: `MainWindow.h`

- [ ] **Step 1: Agregar include de CintaVisualizador**

En `MainWindow.h`, después de la línea `#include "widgets/AvanzadoDialog.h"` (línea 28), agregar:
```cpp
#include "widgets/CintaVisualizador.h"
```

- [ ] **Step 2: Agregar miembros de estado**

En `MainWindow.h`, en la sección `// ── Estado ───────────────────────────────────────────────`, después del miembro `m_brazoState` (línea 197), agregar:
```cpp
    uint8_t  m_velCintaCms   {0};   ///< Última velocidad medida por el MCU (cm/s).
    uint8_t  m_anchoCaja     {10};  ///< Ancho de la caja de referencia en cm.
```

- [ ] **Step 3: Agregar punteros a nuevos widgets**

En `MainWindow.h`, en la sección `// ── Backend ───────────────────────────────────────────────`, después del miembro `m_avanzadoDialog` (línea 179), agregar:
```cpp
    CintaVisualizador *m_visualizadorDialog {nullptr}; ///< Visualizador de cinta en tiempo real.
```

En `MainWindow.h`, en la sección `// ── Tab Monitor – control ───────────────────────────────`, después del miembro `m_spinVel` (línea 217), agregar:
```cpp
    QLabel      *m_lblVelCinta  {nullptr}; ///< Muestra la velocidad medida en cm/s.
```

- [ ] **Step 4: Agregar declaraciones de slots**

En `MainWindow.h`, en la sección `private slots:`, después de `onVelocidadChanged` (línea 125), agregar:
```cpp
    /** @brief Actualiza el label de velocidad y notifica al visualizador. */
    void onVelocidadCintaActualizada(uint8_t vel);

    /** @brief Abre o trae al frente el visualizador de cinta. */
    void onOpenVisualizador();
```

---

## Task 5: Crear `widgets/CintaVisualizador.h`

**Files:**
- Create: `widgets/CintaVisualizador.h`

- [ ] **Step 1: Escribir el header completo**

Crear `widgets/CintaVisualizador.h` con el siguiente contenido:

```cpp
/**
 * @file CintaVisualizador.h
 * @brief Ventana no bloqueante con animación 2D lateral de la cinta transportadora.
 */
#pragma once

#include <QDialog>
#include <QWidget>
#include <QTimer>
#include <QList>
#include <QElapsedTimer>
#include <cstdint>

/** @brief Representa una caja actualmente en la cinta. */
struct CajaEnCinta {
    float   posX     {0.0f}; ///< Posición en cm desde IR0.
    uint8_t tipo     {0};    ///< 6=Pequeña, 8=Mediana, 10=Grande, 0=desconocido.
    uint8_t alturaCm {0};    ///< Altura medida por el HC-SR04.
};

/** @brief Estado de animación de un brazo servo. */
struct BrazoAnim {
    bool activo   {false};
    int  msQuedan {0};  ///< Ms restantes de la animación de extensión.
};

/**
 * @class CintaCanvas
 * @brief Widget que dibuja la cinta, sensores, cajas y brazos a 30 fps con QPainter.
 */
class CintaCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit CintaCanvas(QWidget *parent = nullptr);

public slots:
    /** @brief Crea una caja nueva en posX=0 con la altura indicada. */
    void onCajaMedida(uint8_t alturaCm);

    /**
     * @brief Ancla la caja más cercana a la posición del sensor.
     * outNum 0/1/2 → dist[0/1/2].  outNum 3 = IR0, se ignora.
     */
    void onSensorIr(uint8_t outNum, bool activo);

    /** @brief Inicia animación del brazo servoIdx (400 ms). */
    void onBrazoActuado(uint8_t servoIdx);

    /** @brief Actualiza la velocidad de movimiento de las cajas. */
    void onVelocidadActualizada(uint8_t vel);

    /** @brief Actualiza las distancias S0→salida en cm. */
    void setDistancias(uint8_t d0, uint8_t d1, uint8_t d2);

    /** @brief Actualiza el ancho visual de las cajas. */
    void setAnchoCaja(uint8_t ancho);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    QList<CajaEnCinta> m_cajas;
    BrazoAnim          m_brazos[3];
    float              m_velCmS    {0.0f};
    uint8_t            m_dist[3]   {30, 60, 90}; ///< cm desde IR0 a IR1/2/3.
    uint8_t            m_anchoCaja {10};
    QElapsedTimer      m_elapsed;
    QTimer             m_timer;

    float   pixPerCm()                const;
    float   beltY()                   const;
    QColor  colorForTipo(uint8_t tipo) const;
    QString letraForTipo(uint8_t tipo) const;
};

/**
 * @class CintaVisualizador
 * @brief Diálogo flotante no modal que contiene el CintaCanvas.
 */
class CintaVisualizador : public QDialog
{
    Q_OBJECT
public:
    explicit CintaVisualizador(QWidget *parent = nullptr);

    /** @brief Devuelve el canvas para conectar slots externos. */
    CintaCanvas *canvas() const { return m_canvas; }

    /** @brief Aplica tema oscuro o claro. */
    void setDarkMode(bool dark);

private:
    CintaCanvas *m_canvas {nullptr};
    void applyDarkStyle();
    void applyLightStyle();
};
```

---

## Task 6: Crear `widgets/CintaVisualizador.cpp`

**Files:**
- Create: `widgets/CintaVisualizador.cpp`

- [ ] **Step 1: Escribir la implementación completa**

Crear `widgets/CintaVisualizador.cpp` con el siguiente contenido:

```cpp
/**
 * @file CintaVisualizador.cpp
 * @brief Implementación del visualizador 2D lateral de la cinta transportadora.
 */
#include "CintaVisualizador.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QtMath>

// ─── CintaCanvas ──────────────────────────────────────────────────────────

CintaCanvas::CintaCanvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(600, 160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_elapsed.start();
    connect(&m_timer, &QTimer::timeout, this, &CintaCanvas::tick);
    m_timer.start(33); // ~30 fps
}

float CintaCanvas::pixPerCm() const
{
    const float totalCm = static_cast<float>(m_dist[2])
                        + static_cast<float>(m_anchoCaja) * 4.0f + 10.0f;
    return static_cast<float>(width() - 60) / totalCm;
}

float CintaCanvas::beltY() const
{
    return static_cast<float>(height()) * 0.58f;
}

QColor CintaCanvas::colorForTipo(uint8_t tipo) const
{
    switch (tipo) {
        case 6:  return QColor("#22c55e"); // verde  — Pequeña
        case 8:  return QColor("#3b82f6"); // azul   — Mediana
        case 10: return QColor("#f59e0b"); // naranja — Grande
        default: return QColor("#9ca3af"); // gris   — desconocido
    }
}

QString CintaCanvas::letraForTipo(uint8_t tipo) const
{
    switch (tipo) {
        case 6:  return "P";
        case 8:  return "M";
        case 10: return "G";
        default: return "?";
    }
}

void CintaCanvas::tick()
{
    const float dt = static_cast<float>(m_elapsed.restart()) / 1000.0f;

    for (auto &c : m_cajas)
        c.posX += m_velCmS * dt;

    const float exitCm = static_cast<float>(m_dist[2]) + static_cast<float>(m_anchoCaja) * 2.0f;
    m_cajas.removeIf([exitCm](const CajaEnCinta &c){ return c.posX > exitCm; });

    for (auto &b : m_brazos) {
        if (b.activo) {
            b.msQuedan -= static_cast<int>(dt * 1000.0f);
            if (b.msQuedan <= 0) b.activo = false;
        }
    }

    update();
}

void CintaCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const float ppc     = pixPerCm();
    const float bY      = beltY();
    const float bH      = 20.0f;
    const float origX   = 30.0f;
    const float rollerR = 13.0f;

    // ── Fondo ──────────────────────────────────────────────────
    p.fillRect(rect(), palette().window());

    // ── Cinta ──────────────────────────────────────────────────
    const float beltW = static_cast<float>(width()) - 60.0f;
    QRectF beltRect(origX, bY, beltW, bH);
    p.setBrush(QColor("#4b5563"));
    p.setPen(QPen(QColor("#6b7280"), 1.0));
    p.drawRoundedRect(beltRect, 3, 3);

    // ── Rodillos ───────────────────────────────────────────────
    p.setBrush(QColor("#9ca3af"));
    p.setPen(QPen(QColor("#d1d5db"), 1.5));
    p.drawEllipse(QPointF(origX, bY + bH / 2.0f), rollerR, rollerR);
    p.drawEllipse(QPointF(static_cast<float>(width()) - 30.0f, bY + bH / 2.0f),
                  rollerR, rollerR);

    // ── Posiciones de sensores ──────────────────────────────────
    const float sensorX[4] = {
        origX,
        origX + static_cast<float>(m_dist[0]) * ppc,
        origX + static_cast<float>(m_dist[1]) * ppc,
        origX + static_cast<float>(m_dist[2]) * ppc
    };
    const char *sensorLabels[4] = {"IR0", "IR1\nS0", "IR2\nS1", "IR3\nS2"};

    QFont smallFont = font();
    smallFont.setPointSize(7);
    p.setFont(smallFont);

    for (int i = 0; i < 4; i++) {
        p.setPen(QPen(QColor("#60a5fa"), 1.2, Qt::DashLine));
        p.drawLine(QPointF(sensorX[i], bY - 32.0f), QPointF(sensorX[i], bY));
        p.setPen(QColor("#93c5fd"));
        p.drawText(QRectF(sensorX[i] - 14.0f, bY - 52.0f, 28.0f, 22.0f),
                   Qt::AlignCenter, QString(sensorLabels[i]));
    }

    // ── Brazos activos ──────────────────────────────────────────
    for (int i = 0; i < 3; i++) {
        if (!m_brazos[i].activo) continue;
        const float frac = static_cast<float>(m_brazos[i].msQuedan) / 400.0f;
        const float armH = frac * 24.0f;
        const float ax   = sensorX[i + 1];
        p.setBrush(QColor("#f97316"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(ax - 5.0f, bY + bH, 10.0f, armH), 2, 2);
    }

    // ── Cajas ──────────────────────────────────────────────────
    QFont boxFont = font();
    boxFont.setPointSize(8);
    boxFont.setBold(true);
    p.setFont(boxFont);

    for (const auto &c : m_cajas) {
        const float boxH = 8.0f + static_cast<float>(c.alturaCm) * 0.9f;
        const float boxW = static_cast<float>(m_anchoCaja) * ppc;
        const float bx   = origX + c.posX * ppc - boxW / 2.0f;
        const float by   = bY - boxH;
        const QColor col = colorForTipo(c.tipo);

        p.setBrush(col);
        p.setPen(QPen(col.darker(140), 1.0));
        p.drawRoundedRect(QRectF(bx, by, boxW, boxH), 3, 3);

        p.setPen(Qt::white);
        p.drawText(QRectF(bx, by, boxW, boxH), Qt::AlignCenter, letraForTipo(c.tipo));
    }

    // ── Label de velocidad ─────────────────────────────────────
    QFont velFont = font();
    velFont.setPointSize(8);
    p.setFont(velFont);
    p.setPen(palette().text().color());
    const QString velText = (m_velCmS > 0.0f)
        ? QString("vel: %1 cm/s").arg(static_cast<int>(m_velCmS))
        : QString("vel: --");
    p.drawText(QRectF(origX, static_cast<float>(height()) - 20.0f, 160.0f, 18.0f),
               Qt::AlignLeft | Qt::AlignVCenter, velText);
}

void CintaCanvas::onCajaMedida(uint8_t alturaCm)
{
    CajaEnCinta c;
    c.posX     = 0.0f;
    c.alturaCm = alturaCm;
    // inferir tipo desde la altura usando los umbrales estándar
    if      (alturaCm >= 18) c.tipo = 10;
    else if (alturaCm >= 14) c.tipo = 8;
    else if (alturaCm >= 10) c.tipo = 6;
    else                     c.tipo = 0;
    m_cajas.append(c);
}

void CintaCanvas::onSensorIr(uint8_t outNum, bool activo)
{
    if (!activo || outNum > 2) return; // outNum 3 = IR0, cubierto por cajaMedida

    const float targetPos = static_cast<float>(m_dist[outNum]);
    const float snapLimit = static_cast<float>(m_dist[0]) * 0.6f;

    CajaEnCinta *nearest  = nullptr;
    float        minDist  = 1e6f;
    for (auto &c : m_cajas) {
        const float d = qAbs(c.posX - targetPos);
        if (d < minDist) { minDist = d; nearest = &c; }
    }
    if (nearest && minDist < snapLimit)
        nearest->posX = targetPos;
}

void CintaCanvas::onBrazoActuado(uint8_t servoIdx)
{
    if (servoIdx > 2) return;
    m_brazos[servoIdx].activo   = true;
    m_brazos[servoIdx].msQuedan = 400;
}

void CintaCanvas::onVelocidadActualizada(uint8_t vel)
{
    m_velCmS = static_cast<float>(vel);
}

void CintaCanvas::setDistancias(uint8_t d0, uint8_t d1, uint8_t d2)
{
    m_dist[0] = d0;
    m_dist[1] = d1;
    m_dist[2] = d2;
}

void CintaCanvas::setAnchoCaja(uint8_t ancho)
{
    m_anchoCaja = ancho;
}

// ─── CintaVisualizador ─────────────────────────────────────────────────────

CintaVisualizador::CintaVisualizador(QWidget *parent)
    : QDialog(parent)
    , m_canvas(new CintaCanvas(this))
{
    setWindowTitle("Visualizador de cinta");
    setModal(false);
    setMinimumSize(700, 220);
    resize(900, 240);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(m_canvas);

    applyLightStyle();
}

void CintaVisualizador::setDarkMode(bool dark)
{
    if (dark) applyDarkStyle(); else applyLightStyle();
    m_canvas->update();
}

void CintaVisualizador::applyDarkStyle()
{
    setStyleSheet("QDialog { background: #1f2937; }");
    QPalette pal = m_canvas->palette();
    pal.setColor(QPalette::Window, QColor("#1f2937"));
    pal.setColor(QPalette::Text,   QColor("#f9fafb"));
    m_canvas->setPalette(pal);
    m_canvas->setAutoFillBackground(true);
}

void CintaVisualizador::applyLightStyle()
{
    setStyleSheet("QDialog { background: #f9fafb; }");
    QPalette pal = m_canvas->palette();
    pal.setColor(QPalette::Window, QColor("#f9fafb"));
    pal.setColor(QPalette::Text,   QColor("#111827"));
    m_canvas->setPalette(pal);
    m_canvas->setAutoFillBackground(true);
}
```

- [ ] **Step 2: Compilar para verificar que no hay errores de sintaxis**

En Qt Creator → Build → Build Project.
Expected: 0 errores (el .pro aún no incluye el archivo, por eso este paso es solo para verificar que el código compila cuando se incluya en Task 8).

---

## Task 7: MainWindow.cpp — label, botón, slots, conexiones

**Files:**
- Modify: `MainWindow.cpp`

- [ ] **Step 1: Agregar label `m_lblVelCinta` al panel de control**

En `MainWindow.cpp`, en `buildPanelControl()`, reemplazar el bloque del SpinBox de velocidad (líneas 367–375):
```cpp
    auto *hl = new QHBoxLayout;
    hl->addWidget(new QLabel("Vel:"));
    m_spinVel = new QSpinBox;
    m_spinVel->setRange(1, 10);
    m_spinVel->setValue(5);
    m_spinVel->setToolTip("Velocidad 1–10  (se envía como v×10 al MCU)");
    connect(m_spinVel, &QSpinBox::valueChanged, this, &MainWindow::onVelocidadChanged);
    hl->addWidget(m_spinVel);
    vl->addLayout(hl);
```

Por:
```cpp
    auto *hl = new QHBoxLayout;
    hl->addWidget(new QLabel("Vel:"));
    m_spinVel = new QSpinBox;
    m_spinVel->setRange(1, 10);
    m_spinVel->setValue(5);
    m_spinVel->setToolTip("Velocidad 1–10  (se envía como v×10 al MCU)");
    connect(m_spinVel, &QSpinBox::valueChanged, this, &MainWindow::onVelocidadChanged);
    hl->addWidget(m_spinVel);
    vl->addLayout(hl);

    m_lblVelCinta = new QLabel("Vel. cinta: -- cm/s");
    m_lblVelCinta->setObjectName("lblVelCinta");
    m_lblVelCinta->setStyleSheet("font-size: 11px; color: #9CA3AF;");
    vl->addWidget(m_lblVelCinta);
```

- [ ] **Step 2: Agregar botón "Visualizar" en la toolbar**

En `MainWindow.cpp`, en `buildToolbar()`, después de `tb->addWidget(btnAvanz)` (línea 249) y antes de `addToolBar(tb)` (línea 251):
```cpp
    auto *btnVis = new QPushButton("Visualizar");
    btnVis->setToolTip("Visualizador en tiempo real de la cinta transportadora");
    connect(btnVis, &QPushButton::clicked, this, &MainWindow::onOpenVisualizador);
    tb->addWidget(btnVis);
```

- [ ] **Step 3: Implementar `onVelocidadCintaActualizada`**

En `MainWindow.cpp`, después de `onBrazoActuado` (después de la línea ~660), agregar:
```cpp
void MainWindow::onVelocidadCintaActualizada(uint8_t vel)
{
    m_velCintaCms = vel;
    m_lblVelCinta->setText(QString("Vel. cinta: %1 cm/s").arg(vel));
    if (m_visualizadorDialog)
        m_visualizadorDialog->canvas()->onVelocidadActualizada(vel);
}
```

- [ ] **Step 4: Implementar `onOpenVisualizador`**

En `MainWindow.cpp`, después de `onOpenAvanzado` (después de la línea ~826), agregar:
```cpp
void MainWindow::onOpenVisualizador()
{
    if (!m_visualizadorDialog) {
        m_visualizadorDialog = new CintaVisualizador(this);
        m_visualizadorDialog->setDarkMode(m_darkMode);
        auto *cv = m_visualizadorDialog->canvas();
        cv->setDistancias(m_ciegoCfg.dist_s0[0],
                          m_ciegoCfg.dist_s0[1],
                          m_ciegoCfg.dist_s0[2]);
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

- [ ] **Step 5: Actualizar `onAnchoCajaRequested` para guardar `m_anchoCaja`**

En `MainWindow.cpp`, reemplazar `onAnchoCajaRequested` (líneas 780–788):
```cpp
void MainWindow::onAnchoCajaRequested(uint8_t anchoCm)
{
    if (!m_serial->isOpen()) {
        statusMsg("Sin conexión serial — no se puede enviar.", 3000);
        return;
    }
    m_serial->sendAnchoCaja(anchoCm);
    statusMsg(QString("Ancho de caja enviado: %1 cm (CMD 0x62)").arg(anchoCm));
}
```

Por:
```cpp
void MainWindow::onAnchoCajaRequested(uint8_t anchoCm)
{
    if (!m_serial->isOpen()) {
        statusMsg("Sin conexión serial — no se puede enviar.", 3000);
        return;
    }
    m_anchoCaja = anchoCm;
    m_serial->sendAnchoCaja(anchoCm);
    if (m_visualizadorDialog)
        m_visualizadorDialog->canvas()->setAnchoCaja(anchoCm);
    statusMsg(QString("Ancho de caja enviado: %1 cm (CMD 0x62)").arg(anchoCm));
}
```

- [ ] **Step 6: Agregar tema al visualizador en `toggleDarkMode`**

En `MainWindow.cpp`, en `toggleDarkMode` (líneas 836–839), agregar la línea del visualizador:
```cpp
    if (m_configDialog)        m_configDialog->setDarkMode(dark);
    if (m_velocidadDialog)     m_velocidadDialog->setDarkMode(dark);
    if (m_avanzadoDialog)      m_avanzadoDialog->setDarkMode(dark);
    if (m_visualizadorDialog)  m_visualizadorDialog->setDarkMode(dark);
    m_ledAlive->setDarkMode(dark);
```

- [ ] **Step 7: Conectar `velocidadCintaActualizada` en el constructor**

En `MainWindow.cpp`, en el constructor (líneas 160–167), agregar al final del bloque de connects:
```cpp
    connect(m_serial, &SerialManager::velocidadCintaActualizada,
            this,     &MainWindow::onVelocidadCintaActualizada);
```

---

## Task 8: Registrar `CintaVisualizador` en `.pro`

**Files:**
- Modify: `HMI_Clasificador.pro`

- [ ] **Step 1: Agregar a SOURCES**

En `HMI_Clasificador.pro`, en el bloque `SOURCES +=`, agregar después de `widgets/AvanzadoDialog.cpp`:
```
    widgets/CintaVisualizador.cpp \
```

- [ ] **Step 2: Agregar a HEADERS**

En `HMI_Clasificador.pro`, en el bloque `HEADERS +=`, agregar después de `widgets/AvanzadoDialog.h`:
```
    widgets/CintaVisualizador.h \
```

El archivo `.pro` final debe quedar:
```
QT += core gui widgets serialport

CONFIG += c++17

TARGET   = HMI_Clasificador
TEMPLATE = app

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    comunicacion/UnerProtocol.cpp \
    comunicacion/SerialManager.cpp \
    widgets/LedAliveWidget.cpp \
    widgets/ConfigDialog.cpp \
    widgets/VelocidadDialog.cpp \
    widgets/AvanzadoDialog.cpp \
    widgets/CintaVisualizador.cpp

HEADERS += \
    MainWindow.h \
    comunicacion/UnerProtocol.h \
    comunicacion/SerialManager.h \
    widgets/LedAliveWidget.h \
    widgets/ConfigDialog.h \
    widgets/VelocidadDialog.h \
    widgets/AvanzadoDialog.h \
    widgets/CintaVisualizador.h

QMAKE_CXXFLAGS += -Wall -Wextra

# Linux:   sudo usermod -aG dialout $USER  (luego cerrar sesión)
# Windows: instalar driver CH340 / CP210x si usás Arduino clone
```

- [ ] **Step 3: Build completo**

En Qt Creator → Build → Rebuild Project (qmake + make).
Expected: 0 errores.

- [ ] **Step 4: Commit HMI completo**

```bash
git add widgets/CintaVisualizador.h widgets/CintaVisualizador.cpp
git add MainWindow.h MainWindow.cpp
git add HMI_Clasificador.pro
git commit -m "feat(hmi): add belt visualizer (CintaVisualizador) and velocity label"
```

---

## Task 9: Verificación visual y PR

**Files:**
- (ninguno — verificación + git)

- [ ] **Step 1: Ejecutar el HMI y abrir el visualizador**

Lanzar el ejecutable desde Qt Creator (F5 o Run).
1. Verificar que aparece botón "Visualizar" en la toolbar.
2. Hacer click → debe abrirse una ventana no bloqueante con la cinta dibujada.
3. Verificar que la ventana principal sigue siendo operable (mover, redimensionar, hacer click en otros botones).

- [ ] **Step 2: Verificar label de velocidad**

1. Conectar a un MCU (o usar el simulador si está disponible).
2. Enviar ancho de caja desde el diálogo "Ancho de caja".
3. Pasar una caja por IR0.
4. Verificar que el label "Vel. cinta: X cm/s" en el panel Control se actualiza.

- [ ] **Step 3: Verificar animación de cajas**

1. Con el sistema corriendo y el visualizador abierto.
2. Cada caja detectada (CMD 0x5F) debe aparecer como rectángulo coloreado en el extremo izquierdo de la cinta y moverse hacia la derecha.
3. Al activarse un brazo (CMD 0x52), debe aparecer brevemente un rectángulo naranja bajo el sensor correspondiente.

- [ ] **Step 4: Verificar tema oscuro**

1. Activar "Modo oscuro" en la toolbar.
2. Verificar que el visualizador cambia de fondo blanco a #1f2937.

- [ ] **Step 5: Abrir PR a master**

```bash
git push -u origin feature/velocidad-continua-visualizador
```

Luego crear PR en GitHub desde `feature/velocidad-continua-visualizador` → `master`.
