#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication myApp(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();

    return myApp.exec();
}
