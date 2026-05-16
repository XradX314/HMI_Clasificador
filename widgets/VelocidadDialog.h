#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

// ============================================================
//  VelocidadDialog  –  Medición de velocidad de cinta
//
//  Flujo:
//   1. Usuario ingresa ancho de caja en cm
//   2. Presiona "Medir velocidad" → emite requestMedirVelocidad(anchoCm)
//   3. El programa espera hasta 60 s mostrando un contador regresivo
//   4. Al llegar el resultado → velocidadRecibida(vel)
//   5. Si no llega en 60 s → velocidadFallo()
// ============================================================

class VelocidadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VelocidadDialog(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    // Llamar cuando llega velocidadMedida(vel) del SerialManager
    void velocidadRecibida(uint8_t velCmS);

    // Llamar cuando el SerialManager emite velocidadTimeout()
    void velocidadFallo();

signals:
    void requestMedirVelocidad(uint8_t anchoCm);

private slots:
    void onMedirClicked();
    void onTick();                 // actualiza la barra de progreso cada 1 s

private:
    QSpinBox     *m_spinAncho    {nullptr};
    QPushButton  *m_btnMedir     {nullptr};
    QPushButton  *m_btnCerrar    {nullptr};

    QLabel       *m_lblResultado {nullptr};  // muestra el resultado en grande
    QLabel       *m_lblUnidad    {nullptr};  // "cm/s"
    QLabel       *m_lblEstado    {nullptr};  // texto de estado
    QProgressBar *m_progress     {nullptr};  // cuenta regresiva 60→0

    QTimer       *m_tickTimer    {nullptr};  // tick cada 1 s
    int           m_segundosRestantes {0};

    void setBusyState(bool busy);
    void buildUi();
    void applyDarkStyle();
    void applyLightStyle();
};
