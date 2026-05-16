#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QTimer>
#include <QTabWidget>
#include <QAction>

#include "comunicacion/SerialManager.h"
#include "comunicacion/UnerProtocol.h"
#include "widgets/LedAliveWidget.h"
#include "widgets/ConfigDialog.h"
#include "widgets/VelocidadDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    // Conexión
    void onConnectClicked();
    void onConnected(const QString &port);
    void onDisconnected();
    void onConnectionLost();
    void onSerialError(const QString &msg);
    void onRefreshPorts();

    // Frames entrantes
    void onCajaMedida(uint8_t alturaCm);
    void onSensorIrActualizado(uint8_t outNum, bool activo);
    void onBrazoActuado(uint8_t servoIdx);

    // Controles
    void onStartClicked();
    void onStopClicked();
    void onResetClicked();
    void onVelocidadChanged(int value);
    void onConfigApplied(const Uner::ConfigUmbrales &cfg);
    void onOpenConfig();
    void onOpenVelocidad();
    void onRequestMedir(ConfigDialog::Field field);
    void onMedicionLista(uint8_t cm);
    void onMedicionTimeout();
    void onRequestMedirVelocidad(uint8_t anchoCm);
    void onVelocidadMedida(uint8_t velCmS);
    void onVelocidadTimeout();
    void onBlindModeToggled(bool checked);

    // UI
    void toggleDarkMode(bool dark);
    void checkConfigLock();

private:
    // Backend
    SerialManager    *m_serial          {nullptr};
    ConfigDialog     *m_configDialog    {nullptr};
    VelocidadDialog  *m_velocidadDialog {nullptr};

    // Estado
    bool  m_running       {false};
    bool  m_darkMode      {false};
    int   m_cajasEntrada  {0};
    int   m_cajasSalida   {0};
    int   m_cajasTransito {0};
    bool  m_irState[4]    {false, false, false, false};
    bool  m_brazoState[3] {false, false, false};
    Uner::ConfigUmbrales m_config;
    ConfigDialog::Field  m_pendingMedirField {ConfigDialog::Piso};
    uint8_t m_ultimaVelocidad  {0};        // 0 = no medida aún en esta sesión
    bool    m_pendingBlindMode {false};    // esperando medición de velocidad para activar

    // Toolbar
    QComboBox   *m_comboPuerto  {nullptr};
    QComboBox   *m_comboBaud    {nullptr};
    QPushButton *m_btnConectar  {nullptr};
    QAction     *m_actDarkMode  {nullptr};
    QAction     *m_actConfig    {nullptr};

    // Tab: Monitor
    LedAliveWidget *m_ledAlive    {nullptr};
    QLabel         *m_lblConexion {nullptr};

    QPushButton *m_btnStart  {nullptr};
    QPushButton *m_btnStop   {nullptr};
    QPushButton *m_btnReset  {nullptr};
    QPushButton *m_btnBlindMode {nullptr};
    QSpinBox    *m_spinVel   {nullptr};

    QComboBox   *m_comboSalida[3] {nullptr, nullptr, nullptr};
    QPushButton *m_btnAplicar     {nullptr};

    QLabel *m_lblIR[4]            {nullptr, nullptr, nullptr, nullptr};
    QLabel *m_lblBrazo[3]         {nullptr, nullptr, nullptr};
    QLabel *m_lblBrazoEstado[3]   {nullptr, nullptr, nullptr};

    QLabel *m_lblEntradas   {nullptr};
    QLabel *m_lblSalidas    {nullptr};
    QLabel *m_lblTransito   {nullptr};

    // Última caja medida
    QLabel *m_lblUltimaCajaCm   {nullptr};
    QLabel *m_lblUltimaCajaTipo {nullptr};
    QLabel *m_lblUltimaCajaTs   {nullptr};

    // Construcción
    void buildUi();
    void buildToolbar();
    QWidget *buildTabMonitor();
    QWidget *buildPanelConexion();
    QWidget *buildPanelControl();
    QWidget *buildPanelSalidas();
    QWidget *buildPanelSensores();
    QWidget *buildPanelBrazos();
    QWidget *buildPanelContadores();
    QWidget *buildPanelUltimaCaja();

    // Helpers
    void applyTheme();
    void setConnectedState(bool connected);
    void setRunningState(bool running);
    void updateIrLabel(int idx);
    void updateBrazoLabel(int idx);
    void updateContadores();
    void statusMsg(const QString &msg, int ms = 4000);

    static QString tipoCajaStr(uint8_t cm);
    static Uner::TipoCaja comboToTipo(int index);
};
