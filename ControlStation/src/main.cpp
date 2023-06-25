#include "MainWindow.h"
#include "DesktopCam.h"
#include "OutputWatcher.h"

#include <QApplication>
#include <iostream>
#include <thread>
#include <sys/wait.h>

int raspCam();
int raspAxes();
int raspSwitch();

static pid_t pid;
static DesktopCam *desktopCam;

static void atExit()
{
    std::cout << "[Main] Cleaning up and exiting program..." << std::endl;

    delete desktopCam;

    if (kill(pid, SIGKILL) == -1)
        perror("[Main] kill");
    if (wait(NULL) == -1)
        perror("[Main] wait");

    std::cout << "[Main] Done." << std::endl;
}

static void signalHandler(int signum)
{
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    OutputWatcher *stdoutWatcher = new OutputWatcher(STDOUT_FILENO);
    OutputWatcher *stderrWatcher = new OutputWatcher(STDERR_FILENO);
    Console *console = new Console(stdoutWatcher, stderrWatcher);

    new MainWindow(console);
    desktopCam = new DesktopCam();

//    std::thread(raspCam).detach();
//    std::thread(raspAxes).detach();
//    std::thread(raspSwitch).detach();

    pid = fork();

    if (pid == -1)
    {
        perror("[Main] fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        execlp("python3", "python3", "yolo.py", (char*)NULL);
        perror("[Main] execlp");
        _exit(127);
    }

    if (atexit(atExit) != 0)
        std::cerr << "[Main] atexit failed." << std::endl;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGSEGV, signalHandler);

    return a.exec();
}
