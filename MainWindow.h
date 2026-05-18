/**
 * @file MainWindow.h
 * @brief Ventana principal del HMI del clasificador de paquetes.
 *
 * Define la clase MainWindow que orquesta toda la interfaz gráfica y
 * la comunicación con el MCU a través del SerialManager.
 */

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
#include <QQueue>

#include "comunicacion/SerialManager.h"
#include "comunicacion/UnerProtocol.h"
#include "widgets/LedAliveWidget.h"
#include "widgets/ConfigDialog.h"
#include "widgets/VelocidadDialog.h"
#include "widgets/AvanzadoDialog.h"

/**
 * @class MainWindow
 * @brief Ventana principal del sistema HMI Clasificador de Paquetes.
 *
 * Implementa la capa de presentación completa del HMI.  Gestiona:
 * - La barra de herramientas de conexión serie.
 * - El panel de control (Start/Stop/Reset/Modo ciego/Trigger).
 * - Los paneles de visualización (sensores IR, brazos, contadores,
 *   última caja medida).
 * - La apertura de diálogos de configuración (calibración, ancho de
 *   caja, configuración avanzada de hardware).
 * - La propagación de eventos del SerialManager a la interfaz.
 *
 * Los temas claro/oscuro se aplican mediante hojas de estilo QSS
 * definidas como constantes estáticas al inicio del archivo .cpp.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Construye la ventana principal.
     * @param parent Objeto padre Qt (o nullptr).
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /** @brief Destructor por defecto. */
    ~MainWindow() override = default;

private slots:
    // ── Conexión serie ─────────────────────────────────────────

    /** @brief Alterna entre conectar y desconectar el puerto serie. */
    void onConnectClicked();

    /**
     * @brief Actualiza la UI al establecerse la conexión.
     * @param port Nombre del puerto abierto.
     */
    void onConnected(const QString &port);

    /** @brief Actualiza la UI al cerrarse el puerto serie. */
    void onDisconnected();

    /**
     * @brief Muestra alerta cuando el watchdog de heartbeat expira.
     */
    void onConnectionLost();

    /**
     * @brief Muestra el error en la barra de estado.
     * @param msg Mensaje de error del puerto serie.
     */
    void onSerialError(const QString &msg);

    /** @brief Refresca la lista de puertos disponibles en el ComboBox. */
    void onRefreshPorts();

    // ── Frames entrantes ───────────────────────────────────────

    /**
     * @brief Actualiza contadores y panel de última caja al recibir 0x5F.
     * @param alturaCm Altura de la caja medida en cm.
     */
    void onCajaMedida(uint8_t alturaCm);

    /**
     * @brief Actualiza el indicador LED del sensor IR al recibir 0x5E.
     * @param outNum Número de sensor IR (0–3).
     * @param activo Estado del sensor.
     */
    void onSensorIrActualizado(uint8_t outNum, bool activo);

    /**
     * @brief Actualiza el indicador del brazo y los contadores al recibir 0x52.
     * @param servoIdx Índice del servo actuado (0–2).
     */
    void onBrazoActuado(uint8_t servoIdx);

    // ── Controles básicos ──────────────────────────────────────

    /** @brief Envía CMD_START (0x50) con la configuración de salidas actual. */
    void onStartClicked();

    /** @brief Envía CMD_STOP (0x51). */
    void onStopClicked();

    /** @brief Envía CMD_RESET (0x53) y reinicia los contadores. */
    void onResetClicked();

    /**
     * @brief Envía la velocidad de la cinta (0x54) al cambiar el SpinBox.
     * @param value Valor del SpinBox (índice 1–10).
     */
    void onVelocidadChanged(int value);

    /** @brief Envía CMD_TRIGGER (0x61) al MCU. */
    void onTriggerClicked();

    /**
     * @brief Envía CMD_BLIND_DIST (0x60) al alternar el botón Modo Ciego.
     * @param checked true si el modo ciego queda activado.
     */
    void onBlindModeToggled(bool checked);

    // ── Diálogos de configuración ──────────────────────────────

    /** @brief Abre o trae al frente el diálogo de calibración (0x63). */
    void onOpenConfig();

    /** @brief Abre o trae al frente el diálogo de ancho de caja (0x62). */
    void onOpenVelocidad();

    /** @brief Abre o trae al frente el diálogo de configuración avanzada (0x64–0x67). */
    void onOpenAvanzado();

    /**
     * @brief Envía la calibración al MCU cuando el usuario presiona Enviar 0x63.
     * @param cfg Estructura con alturas, tolerancia y tiempos de brazo.
     */
    void onConfigApplied(const Uner::CalibracionCfg &cfg);

    /**
     * @brief Envía el ancho de caja al MCU (0x62).
     * @param anchoCm Ancho de la caja de referencia en cm.
     */
    void onAnchoCajaRequested(uint8_t anchoCm);

    // ── UI general ─────────────────────────────────────────────

    /**
     * @brief Alterna el tema claro/oscuro de toda la aplicación.
     * @param dark true activa el tema oscuro.
     */
    void toggleDarkMode(bool dark);

    /**
     * @brief Bloquea/desbloquea los combos de salida durante el funcionamiento.
     *
     * Impide reconfigurar las salidas mientras la cinta está en marcha.
     */
    void checkConfigLock();

private:
    // ── Backend ───────────────────────────────────────────────
    SerialManager    *m_serial          {nullptr}; ///< Gestor del puerto serie.
    ConfigDialog     *m_configDialog    {nullptr}; ///< Diálogo de calibración (0x63).
    VelocidadDialog  *m_velocidadDialog {nullptr}; ///< Diálogo de ancho de caja (0x62).
    AvanzadoDialog   *m_avanzadoDialog  {nullptr}; ///< Diálogo de config. avanzada (0x64–0x67).

    // ── Estado ───────────────────────────────────────────────
    bool  m_running  {false}; ///< true mientras el sistema está en marcha.
    bool  m_darkMode {false}; ///< true si el tema oscuro está activo.

    /**
     * @brief Contadores por tipo de caja: índice 0=Pequeña, 1=Mediana, 2=Grande.
     * El total se obtiene sumando los tres elementos.
     */
    int m_cajasEntrada[3]  {0, 0, 0}; ///< Cajas entradas por tipo en la sesión.
    int m_cajasSalida[3]   {0, 0, 0}; ///< Cajas eyectadas por tipo en la sesión.
    int m_cajasTransito[3] {0, 0, 0}; ///< Cajas actualmente en la cinta por tipo.

    /** @brief Cola FIFO de tipos (0/1/2) de cajas en tránsito para despacho ordenado. */
    QQueue<int> m_colaTransito;

    bool  m_irState[4]    {false, false, false, false}; ///< Estado de los 4 sensores IR.
    bool  m_brazoState[3] {false, false, false};        ///< Estado de los 3 brazos.

    /** @brief Configuración actual de modo ciego y distancias (0x60). */
    Uner::CiegoDistancias m_ciegoCfg;

    // ── Toolbar ──────────────────────────────────────────────
    QComboBox   *m_comboPuerto  {nullptr}; ///< Selector de puerto COM.
    QComboBox   *m_comboBaud    {nullptr}; ///< Selector de velocidad en baudios.
    QPushButton *m_btnConectar  {nullptr}; ///< Botón Conectar/Desconectar.

    // ── Tab Monitor – conexión ───────────────────────────────
    LedAliveWidget *m_ledAlive    {nullptr}; ///< LED animado de heartbeat.
    QLabel         *m_lblConexion {nullptr}; ///< Label con el puerto o "Desconectado".

    // ── Tab Monitor – control ────────────────────────────────
    QPushButton *m_btnStart     {nullptr}; ///< Botón Start (0x50).
    QPushButton *m_btnStop      {nullptr}; ///< Botón Stop (0x51).
    QPushButton *m_btnReset     {nullptr}; ///< Botón Reset (0x53).
    QPushButton *m_btnBlindMode {nullptr}; ///< Botón checkable Modo Ciego (0x60).
    QPushButton *m_btnTrigger   {nullptr}; ///< Botón Trigger (0x61).
    QSpinBox    *m_spinVel      {nullptr}; ///< SpinBox de velocidad (0x54).
    QSpinBox    *m_spinDist[3]  {nullptr, nullptr, nullptr}; ///< SpinBoxes de distancias S0→salida.

    // ── Tab Monitor – salidas ────────────────────────────────
    QComboBox   *m_comboSalida[3] {nullptr, nullptr, nullptr}; ///< Combo de tipo de caja por salida.
    QPushButton *m_btnAplicar     {nullptr}; ///< Botón Aplicar/Start del panel de salidas.

    // ── Tab Monitor – sensores ───────────────────────────────
    QLabel *m_lblIR[4]          {nullptr, nullptr, nullptr, nullptr}; ///< LED textual de sensores IR.
    QLabel *m_lblBrazo[3]       {nullptr, nullptr, nullptr};          ///< LED textual de brazos.
    QLabel *m_lblBrazoEstado[3] {nullptr, nullptr, nullptr};          ///< Texto "extendido/retraído".

    // ── Tab Monitor – contadores ─────────────────────────────
    QLabel *m_lblEntradas  {nullptr}; ///< Total de cajas entradas.
    QLabel *m_lblSalidas   {nullptr}; ///< Total de cajas salidas.
    QLabel *m_lblTransito  {nullptr}; ///< Total de cajas en tránsito.
    /** @brief Desglose por tipo [0]=Pequeña [1]=Mediana [2]=Grande para cada estado. */
    QLabel *m_lblEntradaTipo[3]  {nullptr, nullptr, nullptr};
    QLabel *m_lblSalidaTipo[3]   {nullptr, nullptr, nullptr};
    QLabel *m_lblTransitoTipo[3] {nullptr, nullptr, nullptr};

    // ── Tab Monitor – última caja ────────────────────────────
    QLabel *m_lblUltimaCajaCm   {nullptr}; ///< Altura en cm de la última caja.
    QLabel *m_lblUltimaCajaTipo {nullptr}; ///< Tipo inferido ("Pequeña", etc.).
    QLabel *m_lblUltimaCajaTs   {nullptr}; ///< Timestamp de la última detección.

    // ── Construcción de la UI ─────────────────────────────────

    /** @brief Construye toda la UI (toolbar + tabs). */
    void buildUi();

    /** @brief Construye la barra de herramientas de conexión. */
    void buildToolbar();

    /**
     * @brief Construye el tab Monitor.
     * @return Widget del tab.
     */
    QWidget *buildTabMonitor();

    /** @brief Construye el panel de heartbeat/conexión. */
    QWidget *buildPanelConexion();

    /** @brief Construye el panel de control (Start/Stop/Reset/Ciego/Trigger/Vel). */
    QWidget *buildPanelControl();

    /** @brief Construye el panel de configuración de salidas. */
    QWidget *buildPanelSalidas();

    /** @brief Construye el panel de sensores IR. */
    QWidget *buildPanelSensores();

    /** @brief Construye el panel de brazos actuadores. */
    QWidget *buildPanelBrazos();

    /** @brief Construye el panel de contadores de sesión. */
    QWidget *buildPanelContadores();

    /** @brief Construye el panel de última caja medida. */
    QWidget *buildPanelUltimaCaja();

    // ── Helpers ──────────────────────────────────────────────

    /** @brief Aplica el tema QSS activo (claro u oscuro). */
    void applyTheme();

    /**
     * @brief Habilita/deshabilita controles según el estado de conexión.
     * @param connected true si el puerto está abierto.
     */
    void setConnectedState(bool connected);

    /**
     * @brief Habilita/deshabilita controles según el estado de marcha.
     * @param running true si el sistema está en marcha.
     */
    void setRunningState(bool running);

    /**
     * @brief Actualiza el color del indicador LED de un sensor IR.
     * @param idx Índice del sensor (0–3).
     */
    void updateIrLabel(int idx);

    /**
     * @brief Actualiza el color y texto del indicador de un brazo.
     * @param idx Índice del brazo (0–2).
     */
    void updateBrazoLabel(int idx);

    /** @brief Actualiza los tres labels de contadores con los valores actuales. */
    void updateContadores();

    /**
     * @brief Muestra un mensaje en la barra de estado.
     * @param msg Texto del mensaje.
     * @param ms  Duración en ms (0 = permanente).
     */
    void statusMsg(const QString &msg, int ms = 4000);

    /**
     * @brief Convierte una altura en cm al nombre del tipo de caja.
     * @param cm Altura medida.
     * @return Cadena con el tipo ("Pequeña", "Mediana", "Grande" o descripción).
     */
    static QString tipoCajaStr(uint8_t cm);

    /**
     * @brief Convierte una altura en cm al índice de tipo: 0=Pequeña, 1=Mediana, 2=Grande.
     * @param cm Altura medida.
     * @return Índice 0–2, o -1 si no corresponde a ningún tipo conocido.
     */
    static int cmToTipoIdx(uint8_t cm);

    /**
     * @brief Convierte el índice del ComboBox de salida al tipo de caja.
     * @param index Índice del combo (0=ninguna, 1=pequeña, 2=mediana, 3=grande).
     * @return Valor de Uner::TipoCaja correspondiente.
     */
    static Uner::TipoCaja comboToTipo(int index);
};
