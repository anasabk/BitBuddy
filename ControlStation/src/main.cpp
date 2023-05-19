#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    new MainWindow();

    return a.exec();
}
