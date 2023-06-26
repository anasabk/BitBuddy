#include "MainWindow.h"
#include "DesktopCam.h"
#include "OutputWatcher.h"
//#include "communication/RaspCam.h"
//#include "communication/RaspAxes.h"
//#include "communication/RaspSwitch.h"

#include <QApplication>
#include <iostream>
#include <signal.h>

static pid_t pid;
static MainWindow *mainWindow;
static OutputWatcher *stdoutWatcher, *stderrWatcher;

static void atExit()
{
    std::cout << "[Main] Cleaning up and exiting program..." << std::endl;

    std::cout << "[Main] Done." << std::endl;

    delete stdoutWatcher;
    delete stderrWatcher;
}

static void signalHandler(int signum)
{
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    stdoutWatcher = new  OutputWatcher(STDOUT_FILENO);
    stderrWatcher = new OutputWatcher(STDERR_FILENO);
    Console *console = new Console(stdoutWatcher, stderrWatcher);

    new MainWindow(console);
    DesktopCam desktopCam;

//    RaspCam raspCam;
//    RaspAxes raspAxes;
//    RaspSwitch raspSwitch;

    if (atexit(atExit) != 0)
        std::cerr << "[Main] atexit failed." << std::endl;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGSEGV, signalHandler);

    return a.exec();
}
