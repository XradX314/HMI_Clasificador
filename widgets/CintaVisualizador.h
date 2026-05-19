#pragma once

#include <QDialog>
#include <QWidget>
#include <QTimer>
#include <QList>
#include <QElapsedTimer>
#include <cstdint>

struct CajaEnCinta {
    float   posX     {0.0f};
    uint8_t tipo     {0};
    uint8_t alturaCm {0};
};

struct BrazoAnim {
    bool activo   {false};
    int  msQuedan {0};
};

class CintaCanvas : public QWidget {
    Q_OBJECT
public:
    explicit CintaCanvas(QWidget *parent = nullptr);

public slots:
    void onCajaMedida(uint8_t alturaCm);
    void onSensorIr(uint8_t outNum, bool activo);
    void onBrazoActuado(uint8_t servoIdx);
    void onVelocidadActualizada(uint8_t vel);
    void setDistancias(uint8_t d0, uint8_t d1, uint8_t d2);
    void setAnchoCaja(uint8_t ancho);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    QList<CajaEnCinta> m_cajas;
    BrazoAnim          m_brazos[3];
    float              m_velCmS    {0.0f};
    uint8_t            m_dist[3]   {30, 60, 90};
    uint8_t            m_anchoCaja {10};
    QElapsedTimer      m_elapsed;
    QTimer             m_timer;

    float   pixPerCm() const;
    float   beltY()    const;
    QColor  colorForTipo(uint8_t tipo) const;
    QString letraForTipo(uint8_t tipo) const;
};

class CintaVisualizador : public QDialog {
    Q_OBJECT
public:
    explicit CintaVisualizador(QWidget *parent = nullptr);
    CintaCanvas *canvas() const { return m_canvas; }
    void setDarkMode(bool dark);

private:
    CintaCanvas *m_canvas {nullptr};
    void applyDarkStyle();
    void applyLightStyle();
};
