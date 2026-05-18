/**
 * @file VelocidadDialog.h
 * @brief Diálogo para enviar el ancho de la caja de referencia (CMD 0x62).
 */

#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

/**
 * @class VelocidadDialog
 * @brief Diálogo flotante para configurar el ancho de la caja de referencia.
 *
 * Envía el comando 0x62 con un único byte (anchoCaja en cm).  El MCU
 * utiliza ese valor para calcular internamente la velocidad de la cinta;
 * no se espera respuesta de vuelta.
 *
 * Al presionar «Enviar 0x62» se emite la señal @ref requestAnchoCaja.
 */
class VelocidadDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construye el diálogo.
     * @param parent Objeto padre Qt (o nullptr).
     */
    explicit VelocidadDialog(QWidget *parent = nullptr);

    /**
     * @brief Aplica el tema claro u oscuro al diálogo.
     * @param dark true para tema oscuro.
     */
    void setDarkMode(bool dark);

signals:
    /**
     * @brief Se emite cuando el usuario presiona «Enviar 0x62».
     * @param anchoCm Ancho de la caja de referencia en centímetros.
     */
    void requestAnchoCaja(uint8_t anchoCm);

private slots:
    /** @brief Slot del botón «Enviar 0x62»: emite requestAnchoCaja. */
    void onEnviarClicked();

private:
    QSpinBox    *m_spinAncho  {nullptr}; ///< SpinBox del ancho en cm.
    QPushButton *m_btnEnviar  {nullptr}; ///< Botón «Enviar 0x62».
    QPushButton *m_btnCerrar  {nullptr}; ///< Botón «Cerrar».

    /** @brief Construye la interfaz del diálogo. */
    void buildUi();

    /** @brief Aplica la hoja de estilo del tema oscuro. */
    void applyDarkStyle();

    /** @brief Aplica la hoja de estilo del tema claro. */
    void applyLightStyle();
};
