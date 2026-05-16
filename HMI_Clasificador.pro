QT += core gui widgets serialport

CONFIG += c++17

TARGET   = HMI_Clasificador
TEMPLATE = app

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    comunicacion/UnerProtocol.cpp \
    comunicacion/SerialManager.cpp \
    widgets/LedAliveWidget.cpp \
    widgets/ConfigDialog.cpp \
    widgets/VelocidadDialog.cpp

HEADERS += \
    MainWindow.h \
    comunicacion/UnerProtocol.h \
    comunicacion/SerialManager.h \
    widgets/LedAliveWidget.h \
    widgets/ConfigDialog.h \
    widgets/VelocidadDialog.h

QMAKE_CXXFLAGS += -Wall -Wextra

# Linux:   sudo usermod -aG dialout $USER  (luego cerrar sesión)
# Windows: instalar driver CH340 / CP210x si usás Arduino clone
