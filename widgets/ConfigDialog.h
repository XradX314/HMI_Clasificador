#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "comunicacion/UnerProtocol.h"

// ============================================================
//  ConfigDialog  –  Ventana flotante de configuración
//
//  Filas:
//    Distancia piso  | spinbox | [Medir]
//    Caja Pequeña    | spinbox | [Medir]
//    Caja Mediana    | spinbox | [Medir]
//    Caja Grande     | spinbox | [Medir]
//    Tolerancia ±    | spinbox |
//
//  Cada botón Medir emite requestMedir(fieldId).
//  El resultado llega por medicionRecibida(fieldId, cm).
// ============================================================

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    // IDs de los campos medibles
    enum Field { Piso = 0, Pequenia = 1, Mediana = 2, Grande = 3 };

    explicit ConfigDialog(const Uner::ConfigUmbrales &current,
                          QWidget *parent = nullptr);

    Uner::ConfigUmbrales config() const;
    void setDarkMode(bool dark);

    // Llamar cuando llega medicionLista(cm) del SerialManager
    // fieldId es el que se emitió en requestMedir()
    void medicionRecibida(Field field, uint8_t cm);

    // Llamar si el SerialManager emite medicionTimeout()
    void medicionFallo(Field field);

signals:
    void configApplied(const Uner::ConfigUmbrales &cfg);
    // El usuario presionó Medir para el campo indicado
    void requestMedir(ConfigDialog::Field field);

private:
    struct Row {
        QSpinBox    *spin   {nullptr};
        QPushButton *btn    {nullptr};
        QLabel      *status {nullptr};
    };

    Row m_rowPiso;
    Row m_rowPequenia;
    Row m_rowMediana;
    Row m_rowGrande;

    QSpinBox    *m_spinTolerancia {nullptr};
    QPushButton *m_btnAplicar     {nullptr};
    QPushButton *m_btnCerrar      {nullptr};

    Field m_pendingField {Piso};   // campo esperando respuesta

    void buildUi(const Uner::ConfigUmbrales &c);
    void makeRow(QGridLayout *gl, int row, const QString &label,
                 Row &r, int val, int minV, int maxV,
                 Field field, const QString &tooltip);
    void setBusyState(Field field, bool busy);
    void applyDarkStyle();
    void applyLightStyle();
};
