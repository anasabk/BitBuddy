#include "MainWindow.h"
#include "DesktopCam.h"

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
    delete desktopCam;
    kill(pid, SIGKILL);
    wait(NULL);
}

static void signalHandler(int signum)
{
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    std::thread(raspCam).detach();
//    std::thread(raspAxes).detach();
//    std::thread(raspSwitch).detach();
    desktopCam = new DesktopCam();

    pid = fork();

    if (pid == -1)
    {
        perror("[Main] fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        execlp("python3", "python3", "yolo.py", (char*)NULL);
        perror("[Main] execl");
        _exit(127);
    }

    if (atexit(atExit) != 0)
        std::cerr << "[Main] atexit failed." << std::endl;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGSEGV, signalHandler);

    new MainWindow();

    return a.exec();
}
