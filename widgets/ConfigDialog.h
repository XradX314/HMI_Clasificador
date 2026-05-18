/**
 * @file ConfigDialog.h
 * @brief Diálogo de calibración del clasificador (CMD 0x63).
 */

#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "comunicacion/UnerProtocol.h"

/**
 * @class ConfigDialog
 * @brief Diálogo flotante para configurar la calibración del sensor y los brazos.
 *
 * Genera y envía el comando 0x63 (7 bytes) con los siguientes campos:
 *   - calibracion[0]: distancia del sensor al piso (cm).
 *   - calibracion[1]: altura de la caja pequeña (cm).
 *   - calibracion[2]: altura de la caja mediana (cm).
 *   - calibracion[3]: altura de la caja grande (cm).
 *   - tolerancia: margen de error permitido (cm).
 *   - time_arm_extend: ticks de extensión del brazo (1 tick = 2 ms).
 *   - time_arm_retract: ticks de retracción del brazo (1 tick = 2 ms).
 *
 * Cuando el usuario presiona «Enviar 0x63» se emite la señal
 * @ref configApplied con la estructura Uner::CalibracionCfg rellena.
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construye el diálogo inicializado con los valores actuales.
     * @param current Configuración de calibración actual.
     * @param parent  Objeto padre Qt (o nullptr).
     */
    explicit ConfigDialog(const Uner::CalibracionCfg &current,
                          QWidget *parent = nullptr);

    /**
     * @brief Devuelve la configuración de calibración con los valores actuales de los SpinBoxes.
     * @return Estructura Uner::CalibracionCfg lista para enviar al MCU.
     */
    Uner::CalibracionCfg config() const;

    /**
     * @brief Aplica el tema claro u oscuro al diálogo.
     * @param dark true para tema oscuro.
     */
    void setDarkMode(bool dark);

signals:
    /**
     * @brief Se emite cuando el usuario presiona «Enviar 0x63».
     * @param cfg Configuración de calibración a enviar.
     */
    void configApplied(const Uner::CalibracionCfg &cfg);

private:
    QSpinBox    *m_spinPiso          {nullptr}; ///< Distancia al piso (calibracion[0]).
    QSpinBox    *m_spinPequenia      {nullptr}; ///< Altura caja pequeña (calibracion[1]).
    QSpinBox    *m_spinMediana       {nullptr}; ///< Altura caja mediana (calibracion[2]).
    QSpinBox    *m_spinGrande        {nullptr}; ///< Altura caja grande (calibracion[3]).
    QSpinBox    *m_spinTolerancia    {nullptr}; ///< Tolerancia de clasificación.
    QSpinBox    *m_spinArmExtend     {nullptr}; ///< Tiempo de extensión del brazo.
    QSpinBox    *m_spinArmRetract    {nullptr}; ///< Tiempo de retracción del brazo.
    QPushButton *m_btnAplicar        {nullptr}; ///< Botón «Enviar 0x63».
    QPushButton *m_btnCerrar         {nullptr}; ///< Botón «Cerrar».

    /**
     * @brief Construye la interfaz del diálogo.
     * @param c Valores iniciales de los SpinBoxes.
     */
    void buildUi(const Uner::CalibracionCfg &c);

    /** @brief Aplica la hoja de estilo del tema oscuro. */
    void applyDarkStyle();

    /** @brief Aplica la hoja de estilo del tema claro. */
    void applyLightStyle();
};
