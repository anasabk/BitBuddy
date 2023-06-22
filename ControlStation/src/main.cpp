#include "MainWindow.h"

#include <QApplication>
#include <iostream>
#include <thread>
#include <sys/wait.h>

int raspCam();
int raspAxes();
int raspSwitch();
int desktopCam();

static pid_t pid;

static void atExit()
{
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
    std::thread(raspAxes).detach();
//    std::thread(raspSwitch).detach();
    std::thread(desktopCam).detach();

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

    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGSEGV, signalHandler);

    new MainWindow();

    int ret = a.exec();

    return ret;
}
