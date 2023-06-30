#include "MainWindow.h"
//#include "RaspSim/RaspSim.h"

#include <QApplication>
#include <signal.h>

static void signalHandler(int signum)
{
    QApplication::quit();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow::get();
//    RaspSim raspSim(false);

    for (int signum : {SIGINT, SIGTERM, SIGQUIT, SIGABRT, SIGSEGV})
        signal(signum, signalHandler);

    return a.exec();
}
