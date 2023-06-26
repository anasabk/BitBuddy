#include "OutputWatcher.h"

#include <iostream>
#include <fcntl.h>

OutputWatcher::OutputWatcher(int outputFd, QObject *parent) :
    QObject(parent),
    outputFd(outputFd)
{
    thread = std::thread(&OutputWatcher::run, this);
}

OutputWatcher::~OutputWatcher()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    isRunning.store(false);
    thread.join();
}

void OutputWatcher::run()
{
    int originalOutputFd;
    int redirectionFds[2];

    if ((originalOutputFd = dup(outputFd)) == -1)
    {
        perror("[OutputWatcher] dup");
        return;
    }

    if (pipe(redirectionFds) == -1)
    {
        perror("[OutputWatcher] pipe");
        return;
    }

    if (redirectionFds[1] != outputFd)
    {
        if (dup2(redirectionFds[1], outputFd) == -1)
        {
            perror("[OutputWatcher] dup2");
            return;
        }

        if (::close(redirectionFds[1]) == -1)
            perror("[OutputWatcher] close");
    }

    char buffer[OutputWatcher::bufferSize];
    ssize_t bytesRead;

    int flags = fcntl(redirectionFds[0], F_GETFL);
    if (flags == -1)
        perror("[OutputWatcher] fcntl 1");
    if (fcntl(redirectionFds[0], F_SETFL, flags | O_NONBLOCK) == -1)
        perror("[OutputWatcher] fcntl 2");

    while (isRunning.load())
    {
        bytesRead = read(redirectionFds[0], buffer, OutputWatcher::bufferSize - 1);

        if (bytesRead == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("[OutputWatcher] read");
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

            continue;
        }
        else if (bytesRead == 0)
        {
            std::cout << "[OutputWatcher] End of outputFd " << outputFd << std::endl;
            return;
        }
        else
        {
            if (write(originalOutputFd, buffer, bytesRead) == -1)
                perror("[OutputWatcher] write");

            buffer[bytesRead] = '\0';
            emit outputRead(buffer);
        }
    }
}
