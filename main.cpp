/**
 * @file main.cpp
 * @brief Punto de entrada de la aplicación HMI Clasificador de Paquetes.
 *
 * Inicializa la QApplication, crea la MainWindow y arranca el bucle de eventos Qt.
 */
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("HMI Clasificador");
    app.setApplicationVersion("1.1");
    app.setOrganizationName("UNER");

    MainWindow w;
    w.show();
    return app.exec();
}
