#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

class VelocidadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VelocidadDialog(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void velocidadRecibida(uint8_t velCmS);
    void velocidadFallo();

signals:
    void requestMedirVelocidad(uint8_t anchoCm);

private slots:
    void onMedirClicked();
    void onTick();

private:
    QSpinBox     *m_spinAncho         {nullptr};
    QPushButton  *m_btnMedir          {nullptr};
    QPushButton  *m_btnCerrar         {nullptr};
    QLabel       *m_lblResultado      {nullptr};
    QLabel       *m_lblUnidad         {nullptr};
    QLabel       *m_lblEstado         {nullptr};
    QProgressBar *m_progress          {nullptr};
    QTimer       *m_tickTimer         {nullptr};
    int           m_segundosRestantes {0};

    void setBusyState(bool busy);
    void buildUi();
    void applyDarkStyle();
    void applyLightStyle();
};
