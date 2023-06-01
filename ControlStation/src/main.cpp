#include "MainWindow.h"

#include <thread>
#include <QApplication>
#include <QThread>

int cameraServer();
int controlServer();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::thread cameraServ(cameraServer);
    std::thread controlServ(controlServer);
    QThread::sleep(3);
    new MainWindow();

    return a.exec();
}
