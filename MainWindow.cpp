/**
 * @file MainWindow.cpp
 * @brief Implementación de la ventana principal del HMI Clasificador de Paquetes.
 *
 * Contiene la construcción de toda la interfaz gráfica, los slots de control
 * y la lógica de presentación que conecta el SerialManager con la UI.
 */
#include "MainWindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QSizePolicy>
#include <QDateTime>

// =============================================================
//  Temas
// =============================================================

static const QString STYLE_LIGHT = R"(
QMainWindow, QWidget#centralWidget { background: #F3F4F6; }

QToolBar { background: #FFFFFF; border-bottom: 1px solid #E5E7EB; padding: 4px 8px; spacing: 6px; }
QToolBar QLabel { color: #374151; font-size: 13px; }

QTabWidget::pane { border: none; background: transparent; }
QTabBar::tab { background: #E5E7EB; color: #6B7280; padding: 6px 16px;
               border-radius: 4px 4px 0 0; margin-right: 2px; font-size: 13px; }
QTabBar::tab:selected { background: #FFFFFF; color: #111827; font-weight: 500; }
QTabBar::tab:hover:!selected { background: #D1D5DB; }

QGroupBox { background: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px;
            margin-top: 14px; padding-top: 8px;
            font-size: 11px; font-weight: 500; color: #6B7280; }
QGroupBox::title { background: #FFFFFF; subcontrol-origin: margin;
                   subcontrol-position: top left; left: 10px; padding: 0 4px; }

QPushButton { background: #FFFFFF; border: 1px solid #D1D5DB; border-radius: 6px;
              padding: 6px 14px; font-size: 13px; color: #374151; min-height: 28px; }
QPushButton:hover   { background: #F9FAFB; border-color: #9CA3AF; }
QPushButton:pressed { background: #F3F4F6; }
QPushButton:disabled { color: #D1D5DB; border-color: #F3F4F6; background: #FAFAFA; }

QPushButton#btnStart  { background: #ECFDF5; border-color: #6EE7B7; color: #065F46; }
QPushButton#btnStart:hover { background: #D1FAE5; }
QPushButton#btnStart:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }
QPushButton#btnStop   { background: #FEF2F2; border-color: #FCA5A5; color: #991B1B; }
QPushButton#btnStop:hover  { background: #FEE2E2; }
QPushButton#btnStop:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }
QPushButton#btnReset  { background: #FFFBEB; border-color: #FCD34D; color: #92400E; }
QPushButton#btnReset:hover { background: #FEF3C7; }
QPushButton#btnReset:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }
QPushButton#btnConectar { background: #EFF6FF; border-color: #93C5FD; color: #1D4ED8; min-width: 90px; }
QPushButton#btnConectar:hover { background: #DBEAFE; }
QPushButton#btnBlindMode { background: #F5F3FF; border-color: #C4B5FD; color: #5B21B6; }
QPushButton#btnBlindMode:hover { background: #EDE9FE; }
QPushButton#btnBlindMode:checked { background: #7C3AED; border-color: #6D28D9; color: #FFFFFF; }
QPushButton#btnBlindMode:checked:hover { background: #6D28D9; }
QPushButton#btnBlindMode:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }
QPushButton#btnTrigger { background: #FFF7ED; border-color: #FED7AA; color: #C2410C; }
QPushButton#btnTrigger:hover { background: #FFEDD5; }
QPushButton#btnTrigger:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }

QComboBox, QSpinBox { background: #FFFFFF; border: 1px solid #D1D5DB; border-radius: 6px;
                      padding: 4px 8px; font-size: 13px; color: #111827; min-height: 28px; }
QComboBox:focus, QSpinBox:focus { border-color: #93C5FD; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView { background: #FFFFFF; color: #111827; border: 1px solid #E5E7EB; }

QLabel { color: #374151; }
QLabel#lblIR0, QLabel#lblIR1, QLabel#lblIR2, QLabel#lblIR3 { color: #D1D5DB; font-size: 24px; }
QStatusBar { background: #F9FAFB; border-top: 1px solid #E5E7EB; color: #6B7280; font-size: 12px; }

QFrame#cardIR, QFrame#cardBrazo, QFrame#cardCounter, QFrame#cardUltima {
    background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px; }
QPushButton#btnVelAuto, QPushButton#btnVelManual { font-size: 11px; padding: 3px 8px; min-height: 22px; }
QPushButton#btnVelAuto:checked  { background: #ECFDF5; border-color: #6EE7B7; color: #065F46; }
QPushButton#btnVelManual:checked { background: #FFF7ED; border-color: #FED7AA; color: #C2410C; }
QPushButton#btnMedirVel { background: #FFF7ED; border-color: #FED7AA; color: #C2410C; }
QPushButton#btnMedirVel:hover { background: #FFEDD5; }
QPushButton#btnMedirVel:disabled { background: #F9FAFB; color: #D1D5DB; border-color: #F3F4F6; }
)";

static const QString STYLE_DARK = R"(
QMainWindow, QWidget#centralWidget { background: #111827; }

QToolBar { background: #1F2937; border-bottom: 1px solid #374151; padding: 4px 8px; spacing: 6px; }
QToolBar QLabel { color: #D1D5DB; font-size: 13px; }

QTabWidget::pane { border: none; background: transparent; }
QTabBar::tab { background: #1F2937; color: #9CA3AF; padding: 6px 16px;
               border-radius: 4px 4px 0 0; margin-right: 2px; font-size: 13px; }
QTabBar::tab:selected { background: #111827; color: #F9FAFB; font-weight: 500; }
QTabBar::tab:hover:!selected { background: #374151; }

QGroupBox { background: #1F2937; border: 1px solid #374151; border-radius: 8px;
            margin-top: 14px; padding-top: 8px;
            font-size: 11px; font-weight: 500; color: #9CA3AF; }
QGroupBox::title { background: #1F2937; subcontrol-origin: margin;
                   subcontrol-position: top left; left: 10px; padding: 0 4px; }

QPushButton { background: #374151; border: 1px solid #4B5563; border-radius: 6px;
              padding: 6px 14px; font-size: 13px; color: #D1D5DB; min-height: 28px; }
QPushButton:hover   { background: #4B5563; }
QPushButton:pressed { background: #6B7280; }
QPushButton:disabled { color: #4B5563; border-color: #374151; background: #1F2937; }

QPushButton#btnStart  { background: #064E3B; border-color: #065F46; color: #6EE7B7; }
QPushButton#btnStart:hover { background: #065F46; }
QPushButton#btnStart:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
QPushButton#btnStop   { background: #7F1D1D; border-color: #991B1B; color: #FCA5A5; }
QPushButton#btnStop:hover  { background: #991B1B; }
QPushButton#btnStop:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
QPushButton#btnReset  { background: #78350F; border-color: #92400E; color: #FCD34D; }
QPushButton#btnReset:hover { background: #92400E; }
QPushButton#btnReset:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
QPushButton#btnConectar { background: #1E3A5F; border-color: #1D4ED8; color: #93C5FD; min-width: 90px; }
QPushButton#btnConectar:hover { background: #1D4ED8; }
QPushButton#btnBlindMode { background: #2E1065; border-color: #5B21B6; color: #C4B5FD; }
QPushButton#btnBlindMode:hover { background: #3B0764; }
QPushButton#btnBlindMode:checked { background: #7C3AED; border-color: #6D28D9; color: #FFFFFF; }
QPushButton#btnBlindMode:checked:hover { background: #6D28D9; }
QPushButton#btnBlindMode:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
QPushButton#btnTrigger { background: #431407; border-color: #C2410C; color: #FED7AA; }
QPushButton#btnTrigger:hover { background: #7C2D12; }
QPushButton#btnTrigger:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }

QComboBox, QSpinBox { background: #374151; border: 1px solid #4B5563; border-radius: 6px;
                      padding: 4px 8px; font-size: 13px; color: #F9FAFB; min-height: 28px; }
QComboBox:focus, QSpinBox:focus { border-color: #60A5FA; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView { background: #374151; color: #F9FAFB; border: 1px solid #4B5563; }

QLabel { color: #D1D5DB; }
QLabel#lblIR0, QLabel#lblIR1, QLabel#lblIR2, QLabel#lblIR3 { color: #D1D5DB; font-size: 24px; }
QStatusBar { background: #1F2937; border-top: 1px solid #374151; color: #9CA3AF; font-size: 12px; }

QFrame#cardIR, QFrame#cardBrazo, QFrame#cardCounter, QFrame#cardUltima {
    background: #374151; border: 1px solid #4B5563; border-radius: 8px; }
QPushButton#btnVelAuto, QPushButton#btnVelManual { font-size: 11px; padding: 3px 8px; min-height: 22px; }
QPushButton#btnVelAuto:checked  { background: #064E3B; border-color: #065F46; color: #6EE7B7; }
QPushButton#btnVelManual:checked { background: #431407; border-color: #C2410C; color: #FED7AA; }
QPushButton#btnMedirVel { background: #431407; border-color: #C2410C; color: #FED7AA; }
QPushButton#btnMedirVel:hover { background: #7C2D12; }
QPushButton#btnMedirVel:disabled { background: #1F2937; color: #4B5563; border-color: #374151; }
)";

// =============================================================
//  Constructor
// =============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_serial(new SerialManager(this))
{
    setWindowTitle("Clasificador de Paquetes — HMI");
    setMinimumSize(920, 600);
    resize(1040, 700);

    buildUi();
    applyTheme();

    connect(m_serial, &SerialManager::connected,           this, &MainWindow::onConnected);
    connect(m_serial, &SerialManager::disconnected,        this, &MainWindow::onDisconnected);
    connect(m_serial, &SerialManager::connectionLost,      this, &MainWindow::onConnectionLost);
    connect(m_serial, &SerialManager::errorOccurred,       this, &MainWindow::onSerialError);
    connect(m_serial, &SerialManager::aliveReceived,       m_ledAlive, &LedAliveWidget::onAlive);
    connect(m_serial, &SerialManager::cajaMedida,          this, &MainWindow::onCajaMedida);
    connect(m_serial, &SerialManager::sensorIrActualizado, this, &MainWindow::onSensorIrActualizado);
    connect(m_serial, &SerialManager::brazoActuado,               this, &MainWindow::onBrazoActuado);
    connect(m_serial, &SerialManager::velocidadCintaActualizada,  this, &MainWindow::onVelocidadCintaActualizada);

    m_distDebounce = new QTimer(this);
    m_distDebounce->setSingleShot(true);
    m_distDebounce->setInterval(500);
    connect(m_distDebounce, &QTimer::timeout, this, &MainWindow::onDistSendNow);

    setConnectedState(false);
    setRunningState(false);
    onRefreshPorts();
}

// =============================================================
//  buildUi
// =============================================================

void MainWindow::buildUi()
{
    buildToolbar();

    auto *central = new QWidget;
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(0);

    auto *tabs = new QTabWidget;
    tabs->addTab(buildTabMonitor(), "  Monitor  ");
    root->addWidget(tabs);

    statusBar()->showMessage("Desconectado");
}

void MainWindow::buildToolbar()
{
    auto *tb = new QToolBar("Conexión", this);
    tb->setMovable(false);
    tb->setFloatable(false);

    tb->addWidget(new QLabel("Puerto:"));
    m_comboPuerto = new QComboBox;
    m_comboPuerto->setMinimumWidth(130);
    tb->addWidget(m_comboPuerto);

    tb->addWidget(new QLabel("  Baud:"));
    m_comboBaud = new QComboBox;
    for (const QString b : {"9600","19200","38400","57600","115200"})
        m_comboBaud->addItem(b);
    m_comboBaud->setCurrentText("9600");
    tb->addWidget(m_comboBaud);

    auto *btnRefresh = new QPushButton("↻");
    btnRefresh->setFixedWidth(30);
    btnRefresh->setToolTip("Actualizar puertos");
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    tb->addWidget(btnRefresh);

    tb->addSeparator();

    m_btnConectar = new QPushButton("Conectar");
    m_btnConectar->setObjectName("btnConectar");
    connect(m_btnConectar, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    tb->addWidget(m_btnConectar);

    tb->addSeparator();

    auto *btnDark = new QPushButton("☾  Modo oscuro");
    btnDark->setCheckable(true);
    btnDark->setToolTip("Alternar modo oscuro");
    connect(btnDark, &QPushButton::toggled, this, &MainWindow::toggleDarkMode);
    tb->addWidget(btnDark);

    auto *btnCfg = new QPushButton("⚙  Calibración");
    btnCfg->setToolTip("Calibración del clasificador (CMD 0x63)");
    connect(btnCfg, &QPushButton::clicked, this, &MainWindow::onOpenConfig);
    tb->addWidget(btnCfg);

    auto *btnVel = new QPushButton("📦  Ancho de caja");
    btnVel->setToolTip("Enviar ancho de caja de referencia (CMD 0x62)");
    connect(btnVel, &QPushButton::clicked, this, &MainWindow::onOpenVelocidad);
    tb->addWidget(btnVel);

    auto *btnAvanz = new QPushButton("🔧  Config. avanzada");
    btnAvanz->setToolTip("HC-SR04, SG90, debounce IR, timers (0x64–0x67)");
    connect(btnAvanz, &QPushButton::clicked, this, &MainWindow::onOpenAvanzado);
    tb->addWidget(btnAvanz);

    auto *btnVis = new QPushButton("📹  Visualizar");
    btnVis->setToolTip("Abrir visualizador de cinta en tiempo real");
    connect(btnVis, &QPushButton::clicked, this, &MainWindow::onOpenVisualizador);
    tb->addWidget(btnVis);

    addToolBar(tb);
}

// =============================================================
//  Tab Monitor
// =============================================================

QWidget *MainWindow::buildTabMonitor()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    auto *rowTop = new QHBoxLayout;
    rowTop->setSpacing(8);
    rowTop->addWidget(buildPanelConexion(),   0);
    rowTop->addWidget(buildPanelControl(),    0);
    rowTop->addWidget(buildPanelSalidas(),    1);
    rowTop->addWidget(buildPanelUltimaCaja(), 0);
    root->addLayout(rowTop);

    auto *rowMid = new QHBoxLayout;
    rowMid->setSpacing(8);
    rowMid->addWidget(buildPanelSensores(), 1);
    rowMid->addWidget(buildPanelBrazos(),   1);
    root->addLayout(rowMid);

    root->addWidget(buildPanelContadores());

    return page;
}

QWidget *MainWindow::buildPanelConexion()
{
    auto *gb = new QGroupBox("Heartbeat");
    auto *vl = new QVBoxLayout(gb);
    vl->setSpacing(6);
    vl->setContentsMargins(10, 14, 10, 10);

    m_ledAlive = new LedAliveWidget;
    vl->addWidget(m_ledAlive, 0, Qt::AlignHCenter);

    m_lblConexion = new QLabel("Desconectado");
    m_lblConexion->setAlignment(Qt::AlignCenter);
    m_lblConexion->setStyleSheet("font-size: 12px;");
    vl->addWidget(m_lblConexion);

    return gb;
}

QWidget *MainWindow::buildPanelControl()
{
    auto *gb = new QGroupBox("Control");
    auto *vl = new QVBoxLayout(gb);
    vl->setSpacing(4);
    vl->setContentsMargins(10, 14, 10, 10);

    m_btnStart = new QPushButton("▶  Start");
    m_btnStart->setObjectName("btnStart");
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    vl->addWidget(m_btnStart);

    m_btnStop = new QPushButton("■  Stop");
    m_btnStop->setObjectName("btnStop");
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    vl->addWidget(m_btnStop);

    m_btnReset = new QPushButton("↺  Reset");
    m_btnReset->setObjectName("btnReset");
    connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    vl->addWidget(m_btnReset);

    vl->addSpacing(4);

    // Modo ciego (0x60)
    m_btnBlindMode = new QPushButton("Modo Ciego");
    m_btnBlindMode->setObjectName("btnBlindMode");
    m_btnBlindMode->setCheckable(true);
    m_btnBlindMode->setEnabled(false);
    m_btnBlindMode->setToolTip("Activar/desactivar modo ciego (CMD 0x60)");
    connect(m_btnBlindMode, &QPushButton::toggled, this, &MainWindow::onBlindModeToggled);
    vl->addWidget(m_btnBlindMode);

    // Toggle Auto/Manual velocidad
    auto *hlVelToggle = new QHBoxLayout;
    hlVelToggle->setSpacing(4);

    m_btnVelAuto = new QPushButton("Automático");
    m_btnVelAuto->setCheckable(true);
    m_btnVelAuto->setChecked(true);
    m_btnVelAuto->setObjectName("btnVelAuto");
    m_btnVelAuto->setToolTip("Usar velocidad reportada automáticamente por el MCU");

    m_btnVelManual = new QPushButton("Manual");
    m_btnVelManual->setCheckable(true);
    m_btnVelManual->setChecked(false);
    m_btnVelManual->setObjectName("btnVelManual");
    m_btnVelManual->setToolTip("Medir velocidad manualmente con countdown de 60 s");

    hlVelToggle->addWidget(m_btnVelAuto);
    hlVelToggle->addWidget(m_btnVelManual);
    vl->addLayout(hlVelToggle);

    m_btnMedirVel = new QPushButton("⏱  Medir velocidad");
    m_btnMedirVel->setObjectName("btnMedirVel");
    m_btnMedirVel->setVisible(false);
    m_btnMedirVel->setEnabled(false);
    m_btnMedirVel->setToolTip("Abrir diálogo de medición de velocidad con countdown 60 s");
    vl->addWidget(m_btnMedirVel);

    connect(m_btnVelAuto, &QPushButton::clicked, this, [this]() {
        onVelModoToggled(true);
    });
    connect(m_btnVelManual, &QPushButton::clicked, this, [this]() {
        onVelModoToggled(false);
    });
    connect(m_btnMedirVel, &QPushButton::clicked, this, &MainWindow::onOpenVelocidad);

    // Spinboxes de distancias S0→salida[0,1,2]
    const char *distLabels[3] = {"S0→Sal0:", "S0→Sal1:", "S0→Sal2:"};
    const int   distDefaults[3] = {30, 60, 90};
    for (int i = 0; i < 3; i++) {
        auto *hl = new QHBoxLayout;
        hl->setSpacing(4);
        auto *lbl = new QLabel(distLabels[i]);
        lbl->setStyleSheet("font-size: 11px; color: #9CA3AF;");
        hl->addWidget(lbl);
        m_spinDist[i] = new QSpinBox;
        m_spinDist[i]->setRange(0, 255);
        m_spinDist[i]->setValue(distDefaults[i]);
        m_spinDist[i]->setSuffix(" cm");
        m_spinDist[i]->setToolTip(
            QString("Distancia desde S0 a salida %1 (dist_s0_a_salida[%1])").arg(i));
        m_spinDist[i]->setMaximumWidth(90);
        hl->addWidget(m_spinDist[i]);
        connect(m_spinDist[i], &QSpinBox::valueChanged, this, [this]() {
            m_distDebounce->start();
        });
        vl->addLayout(hl);
    }

    vl->addSpacing(4);

    // Trigger (0x61)
    m_btnTrigger = new QPushButton("▶ Trigger");
    m_btnTrigger->setObjectName("btnTrigger");
    m_btnTrigger->setEnabled(false);
    m_btnTrigger->setToolTip("Disparar sensor HC-SR04 (CMD 0x61, sin datos)");
    connect(m_btnTrigger, &QPushButton::clicked, this, &MainWindow::onTriggerClicked);
    vl->addWidget(m_btnTrigger);

    vl->addSpacing(4);

    auto *hl = new QHBoxLayout;
    hl->addWidget(new QLabel("Vel:"));
    m_spinVel = new QSpinBox;
    m_spinVel->setRange(1, 10);
    m_spinVel->setValue(5);
    m_spinVel->setToolTip("Velocidad 1–10  (se envía como v×10 al MCU)");
    connect(m_spinVel, &QSpinBox::valueChanged, this, &MainWindow::onVelocidadChanged);
    hl->addWidget(m_spinVel);
    vl->addLayout(hl);

    auto *hlVel = new QHBoxLayout;
    hlVel->addWidget(new QLabel("Cinta:"));
    m_lblVelCinta = new QLabel("-- cm/s");
    m_lblVelCinta->setStyleSheet("font-size: 11px; color: #9CA3AF;");
    m_lblVelCinta->setToolTip("Velocidad de cinta medida por el MCU (CMD 0x62 MCU→PC)");
    hlVel->addWidget(m_lblVelCinta);
    vl->addLayout(hlVel);

    vl->addStretch();
    return gb;
}

QWidget *MainWindow::buildPanelSalidas()
{
    auto *gb = new QGroupBox("Configuración de salidas");
    auto *gl = new QGridLayout(gb);
    gl->setSpacing(8);
    gl->setContentsMargins(10, 14, 10, 10);
    gl->setColumnStretch(1, 1);

    const QStringList items = {"— ninguna —", "Pequeña (6cm)", "Mediana (8cm)", "Grande (10cm)"};

    for (int i = 0; i < 3; i++) {
        gl->addWidget(new QLabel(QString("Salida %1:").arg(i)), i, 0);
        m_comboSalida[i] = new QComboBox;
        m_comboSalida[i]->addItems(items);
        m_comboSalida[i]->setCurrentIndex(i + 1);
        connect(m_comboSalida[i], &QComboBox::currentIndexChanged, this, &MainWindow::checkConfigLock);
        gl->addWidget(m_comboSalida[i], i, 1);
    }

    m_btnAplicar = new QPushButton("Aplicar (Start)");
    connect(m_btnAplicar, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    gl->addWidget(m_btnAplicar, 3, 0, 1, 2);

    auto *nota = new QLabel("* No reconfigurar con cinta en marcha.");
    nota->setStyleSheet("font-size: 10px; color: #9CA3AF;");
    nota->setWordWrap(true);
    gl->addWidget(nota, 4, 0, 1, 2);

    return gb;
}

QWidget *MainWindow::buildPanelUltimaCaja()
{
    auto *gb = new QGroupBox("Última caja medida");
    auto *vl = new QVBoxLayout(gb);
    vl->setSpacing(6);
    vl->setContentsMargins(14, 16, 14, 12);

    m_lblUltimaCajaCm = new QLabel("– cm");
    m_lblUltimaCajaCm->setAlignment(Qt::AlignCenter);
    m_lblUltimaCajaCm->setStyleSheet(
        "font-size: 36px; font-weight: 600; color: #1D4ED8;");
    vl->addWidget(m_lblUltimaCajaCm);

    m_lblUltimaCajaTipo = new QLabel("–");
    m_lblUltimaCajaTipo->setAlignment(Qt::AlignCenter);
    m_lblUltimaCajaTipo->setStyleSheet(
        "font-size: 14px; font-weight: 500; color: #6B7280;");
    vl->addWidget(m_lblUltimaCajaTipo);

    m_lblUltimaCajaTs = new QLabel("–");
    m_lblUltimaCajaTs->setAlignment(Qt::AlignCenter);
    m_lblUltimaCajaTs->setStyleSheet("font-size: 11px; color: #9CA3AF;");
    vl->addWidget(m_lblUltimaCajaTs);

    vl->addStretch();
    return gb;
}

QWidget *MainWindow::buildPanelSensores()
{
    auto *gb = new QGroupBox("Sensores IR");
    auto *hl = new QHBoxLayout(gb);
    hl->setSpacing(6);
    hl->setContentsMargins(10, 14, 10, 10);

    const QString names[4] = {"S0\ntrigger", "S1\nsalida", "S2\nsalida", "S3\nsalida"};
    for (int i = 0; i < 4; i++) {
        auto *card = new QFrame;
        card->setObjectName("cardIR");
        auto *vl = new QVBoxLayout(card);
        vl->setSpacing(4);
        vl->setContentsMargins(8, 8, 8, 8);

        auto *title = new QLabel(names[i]);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 10px; color: #9CA3AF;");
        vl->addWidget(title);

        m_lblIR[i] = new QLabel("●");
        m_lblIR[i]->setObjectName(QString("lblIR%1").arg(i));
        m_lblIR[i]->setAlignment(Qt::AlignCenter);
        m_lblIR[i]->setStyleSheet("font-size: 24px; color: #D1D5DB;");
        vl->addWidget(m_lblIR[i]);

        hl->addWidget(card, 1);
    }
    return gb;
}

QWidget *MainWindow::buildPanelBrazos()
{
    auto *gb = new QGroupBox("Brazos actuadores");
    auto *hl = new QHBoxLayout(gb);
    hl->setSpacing(6);
    hl->setContentsMargins(10, 14, 10, 10);

    for (int i = 0; i < 3; i++) {
        auto *card = new QFrame;
        card->setObjectName("cardBrazo");
        auto *vl = new QVBoxLayout(card);
        vl->setSpacing(4);
        vl->setContentsMargins(8, 8, 8, 8);

        auto *title = new QLabel(QString("Brazo %1").arg(i));
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 10px; color: #9CA3AF;");
        vl->addWidget(title);

        m_lblBrazo[i] = new QLabel("●");
        m_lblBrazo[i]->setAlignment(Qt::AlignCenter);
        m_lblBrazo[i]->setStyleSheet("font-size: 24px; color: #D1D5DB;");
        vl->addWidget(m_lblBrazo[i]);

        m_lblBrazoEstado[i] = new QLabel("retraído");
        m_lblBrazoEstado[i]->setAlignment(Qt::AlignCenter);
        m_lblBrazoEstado[i]->setStyleSheet("font-size: 10px; color: #9CA3AF;");
        vl->addWidget(m_lblBrazoEstado[i]);

        hl->addWidget(card, 1);
    }
    return gb;
}

QWidget *MainWindow::buildPanelContadores()
{
    auto *gb = new QGroupBox("Contadores de sesión");
    auto *hl = new QHBoxLayout(gb);
    hl->setSpacing(8);
    hl->setContentsMargins(10, 14, 10, 10);

    auto makeCard = [&](const QString &titulo, QLabel *&totalOut, QLabel *tiposOut[3]) {
        auto *card = new QFrame;
        card->setObjectName("cardCounter");
        auto *vl = new QVBoxLayout(card);
        vl->setContentsMargins(14, 10, 14, 10);
        vl->setSpacing(2);

        auto *lbl = new QLabel(titulo);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("font-size: 11px; color: #9CA3AF;");

        totalOut = new QLabel("0");
        totalOut->setAlignment(Qt::AlignCenter);
        totalOut->setStyleSheet("font-size: 28px; font-weight: 600; color: #111827;");

        // Fila con desglose Pequeña / Mediana / Grande
        auto *rowTipos = new QHBoxLayout;
        rowTipos->setSpacing(4);
        const char *tags[3] = {"P: 0", "M: 0", "G: 0"};
        for (int i = 0; i < 3; i++) {
            tiposOut[i] = new QLabel(tags[i]);
            tiposOut[i]->setAlignment(Qt::AlignCenter);
            tiposOut[i]->setStyleSheet("font-size: 11px; color: #6B7280;");
            rowTipos->addWidget(tiposOut[i]);
        }

        vl->addWidget(lbl);
        vl->addWidget(totalOut);
        vl->addLayout(rowTipos);
        return card;
    };

    hl->addWidget(makeCard("entradas",    m_lblEntradas, m_lblEntradaTipo));
    hl->addWidget(makeCard("salidas",     m_lblSalidas,  m_lblSalidaTipo));
    hl->addWidget(makeCard("en tránsito", m_lblTransito, m_lblTransitoTipo));

    return gb;
}

// =============================================================
//  Slots – Conexión serial
// =============================================================

void MainWindow::onConnectClicked()
{
    if (m_serial->isOpen()) { m_serial->close(); return; }
    const QString port = m_comboPuerto->currentText();
    if (port.isEmpty()) { statusMsg("Seleccioná un puerto."); return; }
    m_serial->open(port, m_comboBaud->currentText().toInt());
}

void MainWindow::onConnected(const QString &port)
{
    setConnectedState(true);
    m_lblConexion->setText(port);
    m_lblConexion->setStyleSheet("font-size: 12px; color: #16A34A; font-weight: 500;");
    m_btnConectar->setText("Desconectar");
    statusMsg(QString("Conectado a %1").arg(port));
}

void MainWindow::onDisconnected()
{
    m_btnBlindMode->setChecked(false);
    setConnectedState(false);
    setRunningState(false);
    m_ledAlive->reset();
    m_lblConexion->setText("Desconectado");
    m_lblConexion->setStyleSheet("font-size: 12px; color: #6B7280;");
    m_btnConectar->setText("Conectar");
    statusMsg("Desconectado.");
}

void MainWindow::onConnectionLost()
{
    statusBar()->showMessage("⚠  Sin heartbeat del MCU — verificá la conexión.", 0);
    QMessageBox::warning(this, "Conexión perdida",
        "No se recibió heartbeat del MCU en los últimos 7 segundos.\n"
        "Verificá la conexión USB / cable serial.");
}

void MainWindow::onSerialError(const QString &msg) { statusMsg("Error: " + msg); }

void MainWindow::onRefreshPorts()
{
    const QString cur = m_comboPuerto->currentText();
    m_comboPuerto->clear();
    m_comboPuerto->addItems(SerialManager::availablePorts());
    const int idx = m_comboPuerto->findText(cur);
    if (idx >= 0) m_comboPuerto->setCurrentIndex(idx);
}

// =============================================================
//  Slots – Frames entrantes
// =============================================================

void MainWindow::onCajaMedida(uint8_t alturaCm)
{
    const int idx = cmToTipoIdx(alturaCm);
    if (idx >= 0) {
        m_cajasEntrada[idx]++;
        m_cajasTransito[idx]++;
        m_colaTransito.enqueue(idx);
    }
    updateContadores();

    m_lblUltimaCajaCm->setText(QString("%1 cm").arg(alturaCm));
    m_lblUltimaCajaTipo->setText(tipoCajaStr(alturaCm));
    m_lblUltimaCajaTs->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

    QString col = "#1D4ED8";
    if      (alturaCm == 6)  col = "#16A34A";
    else if (alturaCm == 8)  col = "#D97706";
    else if (alturaCm == 10) col = "#DC2626";

    m_lblUltimaCajaCm->setStyleSheet(
        QString("font-size: 36px; font-weight: 600; color: %1;").arg(col));

    statusMsg(QString("Nueva caja: %1 cm (%2)").arg(alturaCm).arg(tipoCajaStr(alturaCm)));
}

void MainWindow::onSensorIrActualizado(uint8_t outNum, bool activo)
{
    if (outNum < 4) {
        m_irState[outNum] = activo;
        updateIrLabel(outNum);
    }
}

void MainWindow::onBrazoActuado(uint8_t servoIdx)
{
    if (servoIdx >= 3) return;

    m_brazoState[servoIdx] = true;
    updateBrazoLabel(servoIdx);

    if (!m_colaTransito.isEmpty()) {
        const int idx = m_colaTransito.dequeue();
        m_cajasSalida[idx]++;
        if (m_cajasTransito[idx] > 0) m_cajasTransito[idx]--;
    }
    updateContadores();
    statusMsg(QString("Brazo %1 activado — caja eyectada.").arg(servoIdx));

    QTimer::singleShot(500, this, [this, servoIdx]() {
        m_brazoState[servoIdx] = false;
        updateBrazoLabel(servoIdx);
    });
}

// =============================================================
//  Slots – Controles
// =============================================================

void MainWindow::onStartClicked()
{
    if (!m_serial->isOpen()) return;
    m_serial->sendStart(
        comboToTipo(m_comboSalida[0]->currentIndex()),
        comboToTipo(m_comboSalida[1]->currentIndex()),
        comboToTipo(m_comboSalida[2]->currentIndex()));
    setRunningState(true);
    for (int i = 0; i < 3; i++) m_cajasEntrada[i] = m_cajasSalida[i] = m_cajasTransito[i] = 0;
    m_colaTransito.clear();
    updateContadores();
    statusMsg("Sistema iniciado.");
}

void MainWindow::onStopClicked()
{
    if (!m_serial->isOpen()) return;
    m_serial->sendStop();
    setRunningState(false);
    statusMsg("Sistema detenido.");
}

void MainWindow::onResetClicked()
{
    if (!m_serial->isOpen()) return;
    m_serial->sendReset();
    setRunningState(false);
    for (int i = 0; i < 3; i++) m_cajasEntrada[i] = m_cajasSalida[i] = m_cajasTransito[i] = 0;
    m_colaTransito.clear();
    updateContadores();
    for (int i = 0; i < 4; i++) { m_irState[i]   = false; updateIrLabel(i); }
    for (int i = 0; i < 3; i++) { m_brazoState[i] = false; updateBrazoLabel(i); }
    m_lblUltimaCajaCm->setText("– cm");
    m_lblUltimaCajaTipo->setText("–");
    m_lblUltimaCajaTs->setText("–");
    statusMsg("Reset enviado.");
}

void MainWindow::onVelocidadChanged(int value)
{
    if (m_serial->isOpen())
        m_serial->sendVelocidad(static_cast<uint8_t>(value));
}

void MainWindow::onTriggerClicked()
{
    if (!m_serial->isOpen()) return;
    m_serial->sendTrigger();
    statusMsg("Trigger enviado (CMD 0x61).");
}

void MainWindow::onBlindModeToggled(bool checked)
{
    if (!m_serial->isOpen()) {
        m_btnBlindMode->setChecked(false);
        return;
    }
    // Leer las distancias de los spinboxes
    m_ciegoCfg.modo_ciego = checked ? 1 : 0;
    for (int i = 0; i < 3; i++)
        m_ciegoCfg.dist_s0[i] = static_cast<uint8_t>(m_spinDist[i]->value());

    m_serial->sendBlindDist(m_ciegoCfg, 4);
    statusMsg(QString("Modo ciego %1 — dist: %2/%3/%4 cm")
              .arg(checked ? "activado" : "desactivado")
              .arg(m_ciegoCfg.dist_s0[0])
              .arg(m_ciegoCfg.dist_s0[1])
              .arg(m_ciegoCfg.dist_s0[2]));
}

void MainWindow::onDistSendNow()
{
    for (int i = 0; i < 3; i++)
        m_ciegoCfg.dist_s0[i] = static_cast<uint8_t>(m_spinDist[i]->value());
    if (m_serial->isOpen()) {
        m_serial->sendBlindDist(m_ciegoCfg, 4);
        statusMsg(QString("Distancias actualizadas: %1/%2/%3 cm")
                  .arg(m_ciegoCfg.dist_s0[0])
                  .arg(m_ciegoCfg.dist_s0[1])
                  .arg(m_ciegoCfg.dist_s0[2]));
    }
}

void MainWindow::onVelModoToggled(bool autoMode)
{
    m_velModoAuto = autoMode;
    m_btnVelAuto->setChecked(autoMode);
    m_btnVelManual->setChecked(!autoMode);
    m_btnMedirVel->setVisible(!autoMode);
    m_btnMedirVel->setEnabled(!autoMode && m_serial->isOpen());
}

// =============================================================
//  Slots – Diálogos de configuración
// =============================================================

void MainWindow::onOpenConfig()
{
    if (!m_configDialog) {
        Uner::CalibracionCfg defaultCfg;
        m_configDialog = new ConfigDialog(defaultCfg, this);
        m_configDialog->setDarkMode(m_darkMode);
        connect(m_configDialog, &ConfigDialog::configApplied,
                this,           &MainWindow::onConfigApplied);
        connect(m_configDialog, &ConfigDialog::requestMedir,
                this,           &MainWindow::onMedirRequested);
        connect(m_serial,       &SerialManager::medicionRecibida,
                this,           &MainWindow::onMedicionRecibida);
        connect(m_serial,       &SerialManager::medicionTimeout,
                this,           &MainWindow::onMedicionTimeout);
    }
    m_configDialog->show();
    m_configDialog->raise();
    m_configDialog->activateWindow();
}

void MainWindow::onConfigApplied(const Uner::CalibracionCfg &cfg)
{
    if (m_serial->isOpen()) {
        m_serial->sendCalibracion(cfg);
        statusMsg(QString("Calibración enviada 0x63: piso=%1 peq=%2 med=%3 gde=%4 tol=±%5 ext=%6 ret=%7")
                  .arg(cfg.calibracion[0]).arg(cfg.calibracion[1])
                  .arg(cfg.calibracion[2]).arg(cfg.calibracion[3])
                  .arg(cfg.tolerancia)
                  .arg(cfg.time_arm_extend).arg(cfg.time_arm_retract));
    } else {
        statusMsg("Calibración guardada (sin conexión serial).", 3000);
    }
}

void MainWindow::onMedirRequested(ConfigDialog::Field field)
{
    if (!m_serial->isOpen()) {
        statusMsg("Sin conexión serial — no se puede medir.", 3000);
        if (m_configDialog) m_configDialog->medicionFallo(field);
        return;
    }
    m_pendingMedirField = field;
    m_serial->sendMedir();
    statusMsg("Midiendo con HC-SR04 (CMD 0x61)…", 0);
}

void MainWindow::onMedicionRecibida(uint8_t cm)
{
    if (m_configDialog)
        m_configDialog->medicionRecibida(m_pendingMedirField, cm);
    statusMsg(QString("Medición recibida: %1 cm").arg(cm));
}

void MainWindow::onMedicionTimeout()
{
    if (m_configDialog)
        m_configDialog->medicionFallo(m_pendingMedirField);
    statusMsg("Timeout de medición (3 s sin respuesta).", 4000);
}

void MainWindow::onOpenVelocidad()
{
    if (!m_velocidadDialog) {
        m_velocidadDialog = new VelocidadDialog(this);
        m_velocidadDialog->setDarkMode(m_darkMode);
        connect(m_velocidadDialog, &VelocidadDialog::requestMedirVelocidad,
                this,              &MainWindow::onAnchoCajaRequested);
    }
    m_velocidadDialog->show();
    m_velocidadDialog->raise();
    m_velocidadDialog->activateWindow();
}

void MainWindow::onAnchoCajaRequested(uint8_t anchoCm)
{
    if (!m_serial->isOpen()) {
        statusMsg("Sin conexión serial — no se puede enviar.", 3000);
        if (m_velocidadDialog) m_velocidadDialog->velocidadFallo();
        return;
    }
    m_anchoCaja = anchoCm;
    if (m_visualizadorDialog)
        m_visualizadorDialog->canvas()->setAnchoCaja(anchoCm);
    m_serial->sendAnchoCaja(anchoCm);
    statusMsg(QString("Ancho de caja enviado: %1 cm (CMD 0x62) — esperando velocidad…").arg(anchoCm));
}

void MainWindow::onVelocidadCintaActualizada(uint8_t vel)
{
    m_velCintaCms = vel;
    if (m_lblVelCinta)
        m_lblVelCinta->setText(QString("%1 cm/s").arg(vel));
    if (!m_velModoAuto && m_velocidadDialog && m_velocidadDialog->isVisible())
        m_velocidadDialog->velocidadRecibida(vel);
    if (m_visualizadorDialog)
        m_visualizadorDialog->canvas()->onVelocidadActualizada(vel);
}

void MainWindow::onOpenVisualizador()
{
    if (!m_visualizadorDialog) {
        m_visualizadorDialog = new CintaVisualizador(this);
        m_visualizadorDialog->setDarkMode(m_darkMode);
        m_visualizadorDialog->canvas()->setAnchoCaja(m_anchoCaja);
        m_visualizadorDialog->canvas()->setDistancias(
            static_cast<uint8_t>(m_spinDist[0]->value()),
            static_cast<uint8_t>(m_spinDist[1]->value()),
            static_cast<uint8_t>(m_spinDist[2]->value()));
        if (m_velCintaCms > 0)
            m_visualizadorDialog->canvas()->onVelocidadActualizada(m_velCintaCms);

        connect(m_serial, &SerialManager::cajaMedida,
                m_visualizadorDialog->canvas(), &CintaCanvas::onCajaMedida);
        connect(m_serial, &SerialManager::sensorIrActualizado,
                m_visualizadorDialog->canvas(), &CintaCanvas::onSensorIr);
        connect(m_serial, &SerialManager::brazoActuado,
                m_visualizadorDialog->canvas(), &CintaCanvas::onBrazoActuado);
        connect(m_serial, &SerialManager::velocidadCintaActualizada,
                m_visualizadorDialog->canvas(), &CintaCanvas::onVelocidadActualizada);
    }
    m_visualizadorDialog->show();
    m_visualizadorDialog->raise();
    m_visualizadorDialog->activateWindow();
}

void MainWindow::onOpenAvanzado()
{
    if (!m_avanzadoDialog) {
        m_avanzadoDialog = new AvanzadoDialog(this);
        m_avanzadoDialog->setDarkMode(m_darkMode);
        connect(m_avanzadoDialog, &AvanzadoDialog::sendHcsr04,
                this, [this](const Uner::Hcsr04Cfg &cfg) {
            if (!m_serial->isOpen()) { statusMsg("Sin conexión serial.", 3000); return; }
            m_serial->sendHcsr04Cfg(cfg);
            statusMsg(QString("HC-SR04 enviado (0x64): pulse=%1µs timeout=%2µs")
                      .arg(cfg.trig_pulse_us).arg(cfg.timeout_us));
        });
        connect(m_avanzadoDialog, &AvanzadoDialog::sendSg90,
                this, [this](const Uner::Sg90Cfg &cfg) {
            if (!m_serial->isOpen()) { statusMsg("Sin conexión serial.", 3000); return; }
            m_serial->sendSg90Cfg(cfg);
            statusMsg(QString("SG90 enviado (0x65): period=%1 min=%2 max=%3 neutral=%4")
                      .arg(cfg.period_us).arg(cfg.pulse_min_us)
                      .arg(cfg.pulse_max_us).arg(cfg.pulse_neutral_us));
        });
        connect(m_avanzadoDialog, &AvanzadoDialog::sendIrDebounce,
                this, [this](uint8_t db) {
            if (!m_serial->isOpen()) { statusMsg("Sin conexión serial.", 3000); return; }
            m_serial->sendIrDebounce(db);
            statusMsg(QString("Debounce IR enviado (0x66): %1 ticks").arg(db));
        });
        connect(m_avanzadoDialog, &AvanzadoDialog::sendTimers,
                this, [this](const Uner::TimersCfg &cfg) {
            if (!m_serial->isOpen()) { statusMsg("Sin conexión serial.", 3000); return; }
            m_serial->sendTimersCfg(cfg);
            statusMsg("Timers/cajas enviados (CMD 0x67).");
        });
    }
    m_avanzadoDialog->show();
    m_avanzadoDialog->raise();
    m_avanzadoDialog->activateWindow();
}

// =============================================================
//  Tema
// =============================================================

void MainWindow::toggleDarkMode(bool dark)
{
    m_darkMode = dark;
    applyTheme();
    if (m_configDialog)       m_configDialog->setDarkMode(dark);
    if (m_velocidadDialog)    m_velocidadDialog->setDarkMode(dark);
    if (m_avanzadoDialog)     m_avanzadoDialog->setDarkMode(dark);
    if (m_visualizadorDialog) m_visualizadorDialog->setDarkMode(dark);
    m_ledAlive->setDarkMode(dark);
}

void MainWindow::applyTheme()
{
    setStyleSheet(m_darkMode ? STYLE_DARK : STYLE_LIGHT);

    const QString numColor  = m_darkMode ? "#F9FAFB" : "#111827";
    const QString tipoColor = m_darkMode ? "#9CA3AF" : "#6B7280";

    for (QLabel *l : {m_lblEntradas, m_lblSalidas, m_lblTransito})
        if (l) l->setStyleSheet(
            QString("font-size: 28px; font-weight: 600; color: %1;").arg(numColor));

    for (int i = 0; i < 3; i++) {
        const QString ss = QString("font-size: 11px; color: %1;").arg(tipoColor);
        if (m_lblEntradaTipo[i])  m_lblEntradaTipo[i]->setStyleSheet(ss);
        if (m_lblSalidaTipo[i])   m_lblSalidaTipo[i]->setStyleSheet(ss);
        if (m_lblTransitoTipo[i]) m_lblTransitoTipo[i]->setStyleSheet(ss);
    }
}

// =============================================================
//  Helpers de estado UI
// =============================================================

void MainWindow::setConnectedState(bool connected)
{
    m_btnStart->setEnabled(connected && !m_running);
    m_btnStop->setEnabled(false);
    m_btnReset->setEnabled(connected);
    m_spinVel->setEnabled(connected);
    m_btnBlindMode->setEnabled(connected && !m_running);
    m_btnTrigger->setEnabled(connected);
    if (m_btnMedirVel)
        m_btnMedirVel->setEnabled(!m_velModoAuto && connected);
    checkConfigLock();
}

void MainWindow::setRunningState(bool running)
{
    m_running = running;
    m_btnStart->setEnabled(!running && m_serial->isOpen());
    m_btnStop->setEnabled(running);
    m_btnBlindMode->setEnabled(!running && m_serial->isOpen());
    checkConfigLock();
}

void MainWindow::checkConfigLock()
{
    for (int i = 0; i < 3; i++) m_comboSalida[i]->setEnabled(!m_running);
    m_btnAplicar->setEnabled(!m_running);
}

void MainWindow::updateIrLabel(int idx)
{
    const bool a = m_irState[idx];
    const QString name = QString("lblIR%1").arg(idx);
    m_lblIR[idx]->setStyleSheet(
        QString("QLabel#%1 { font-size: 24px; color: %2; }")
            .arg(name, a ? "#10B981" : "#D1D5DB"));
}

void MainWindow::updateBrazoLabel(int idx)
{
    const bool ext = m_brazoState[idx];
    m_lblBrazo[idx]->setStyleSheet(
        ext ? "font-size: 24px; color: #F59E0B;"
            : "font-size: 24px; color: #D1D5DB;");
    m_lblBrazoEstado[idx]->setText(ext ? "extendido" : "retraído");
    m_lblBrazoEstado[idx]->setStyleSheet(
        ext ? "font-size: 10px; color: #D97706;"
            : "font-size: 10px; color: #9CA3AF;");
}

void MainWindow::updateContadores()
{
    static const char *tags[3] = {"P", "M", "G"};

    auto total = [](const int a[3]) { return a[0] + a[1] + a[2]; };

    m_lblEntradas->setText(QString::number(total(m_cajasEntrada)));
    m_lblSalidas ->setText(QString::number(total(m_cajasSalida)));
    m_lblTransito->setText(QString::number(total(m_cajasTransito)));

    for (int i = 0; i < 3; i++) {
        m_lblEntradaTipo[i] ->setText(QString("%1: %2").arg(tags[i]).arg(m_cajasEntrada[i]));
        m_lblSalidaTipo[i]  ->setText(QString("%1: %2").arg(tags[i]).arg(m_cajasSalida[i]));
        m_lblTransitoTipo[i]->setText(QString("%1: %2").arg(tags[i]).arg(m_cajasTransito[i]));
    }
}

void MainWindow::statusMsg(const QString &msg, int ms)
{
    statusBar()->showMessage(msg, ms);
}

// =============================================================
//  Utilidades estáticas
// =============================================================

QString MainWindow::tipoCajaStr(uint8_t cm)
{
    switch (cm) {
    case 6:  return "Pequeña";
    case 8:  return "Mediana";
    case 10: return "Grande";
    default: return QString("Desconocida (%1cm)").arg(cm);
    }
}

int MainWindow::cmToTipoIdx(uint8_t cm)
{
    switch (cm) {
    case 6:  return 0;
    case 8:  return 1;
    case 10: return 2;
    default: return -1;
    }
}

Uner::TipoCaja MainWindow::comboToTipo(int index)
{
    switch (index) {
    case 1: return Uner::TipoCaja::Pequenia;
    case 2: return Uner::TipoCaja::Mediana;
    case 3: return Uner::TipoCaja::Grande;
    default: return Uner::TipoCaja::Ninguna;
    }
}
