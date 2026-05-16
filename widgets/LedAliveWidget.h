#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QDateTime>

// ============================================================
//  LedAliveWidget  –  LED circular de heartbeat 0xF0
//
//  Layout vertical:
//    [ LED  ]     <- paintEvent, centrado
//    estado       <- online / offline / timeout
//    recibidos: N
//    intervalo: N s
//    último: HH:MM:SS
// ============================================================

class LedAliveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LedAliveWidget(QWidget *parent = nullptr);

    void onAlive();
    void reset();
    void setDarkMode(bool dark);

protected:
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void turnOff();
    void onWatchdogTimeout();

private:
    enum class LedState { Off, On, Timeout };

    LedState  m_state   {LedState::Off};
    bool      m_dark    {false};

    QTimer   *m_offTimer  {nullptr};
    QTimer   *m_watchdog  {nullptr};

    int       m_count   {0};
    qint64    m_lastTs  {0};

    // Estos labels están en un QVBoxLayout debajo del LED
    QLabel   *m_labelStatus   {nullptr};
    QLabel   *m_labelCount    {nullptr};
    QLabel   *m_labelInterval {nullptr};
    QLabel   *m_labelLast     {nullptr};

    // Área reservada para el dibujo del LED (top del widget)
    static constexpr int LED_D   = 36;   // diámetro px
    static constexpr int LED_PAD = 8;    // margen sobre/bajo el LED

    void buildLayout();
    void refreshLabelStyles();
    void updateLabels(qint64 intervalMs);
};
