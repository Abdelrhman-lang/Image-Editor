#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // The main application
    QApplication a(argc, argv);

    // Create and open the main window
    MainWindow w;
    w.show();

    // Display the main window
    return a.exec();
}
