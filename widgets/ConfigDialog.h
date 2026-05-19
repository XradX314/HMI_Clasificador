#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "comunicacion/UnerProtocol.h"

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    enum Field { Piso, Pequenia, Mediana, Grande };

    explicit ConfigDialog(const Uner::CalibracionCfg &current,
                          QWidget *parent = nullptr);

    Uner::CalibracionCfg config() const;
    void setDarkMode(bool dark);

    void medicionRecibida(Field field, uint8_t cm);
    void medicionFallo(Field field);

signals:
    void configApplied(const Uner::CalibracionCfg &cfg);
    void requestMedir(ConfigDialog::Field field);

private:
    struct Row {
        QSpinBox    *spin   {nullptr};
        QPushButton *medir  {nullptr};
        QLabel      *status {nullptr};
    };

    Row         m_rowPiso       {};
    Row         m_rowPequenia   {};
    Row         m_rowMediana    {};
    Row         m_rowGrande     {};
    QSpinBox    *m_spinTolerancia   {nullptr};
    QSpinBox    *m_spinArmExtend    {nullptr};
    QSpinBox    *m_spinArmRetract   {nullptr};
    QPushButton *m_btnAplicar       {nullptr};
    QPushButton *m_btnCerrar        {nullptr};

    void buildUi(const Uner::CalibracionCfg &c);
    void makeRow(QGridLayout *gl, int row, const QString &label,
                 Row &r, int val, int minV, int maxV,
                 Field field, const QString &tooltip);
    void setBusyState(Field field, bool busy);
    void applyDarkStyle();
    void applyLightStyle();
};
