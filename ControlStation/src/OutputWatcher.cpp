#include "OutputWatcher.h"

#include <iostream>

#define BUFFER_SIZE 4096

OutputWatcher::OutputWatcher(int outputFd, QObject *parent) :
    QThread(parent),
    outputFd(outputFd)
{
    start();
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

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while (true)
    {
        bytesRead = read(redirectionFds[0], buffer, BUFFER_SIZE - 1);

        if (bytesRead == -1)
            perror("[OutputWatcher] read");
        else if (bytesRead == 0)
        {
            std::cout << "[OutputWatcher] end of outputFd " << outputFd << std::endl;
            return;
        }
        else
        {
            buffer[bytesRead] = '\0';

            char *output = (char*)malloc(bytesRead + 1);
            if (output == NULL)
            {
                std::cerr << "[OutputWatcher] malloc failed." << std::endl;
                continue;
            }
            std::strncpy(output, buffer, bytesRead + 1);

            if (write(originalOutputFd, output, bytesRead) == -1)
                perror("[OutputWatcher] write");

            emit outputRead(output, bytesRead);
        }
    }
}
