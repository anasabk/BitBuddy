#include "OutputWatcher.h"

#include <iostream>

#define BUFFER_SIZE 4096

OutputWatcher::OutputWatcher(int outputFd, QObject *parent) :
    QThread(parent),
    outputFd(outputFd)
{}

void OutputWatcher::run()
{
    int originalOutputFd;
    int outputFds[2];

    if ((originalOutputFd = dup(outputFd)) == -1)
    {
        perror("[MainWindow] dup");
        return;
    }

    if (pipe(outputFds) == -1)
    {
        perror("[MainWindow] pipe");
        return;
    }

    if (dup2(outputFds[1], outputFd) == -1)
    {
        perror("[MainWindow] dup2");
        return;
    }

    if (::close(outputFds[1]) == -1)
        perror("[MainWindow] close");

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while (true)
    {
        bytesRead = read(outputFds[0], buffer, BUFFER_SIZE - 1);

        if (bytesRead == -1)
            perror("[MainWindow] read");
        else if (bytesRead == 0)
        {
            std::cout << "[MainWindow] end of outputFd " << outputFd << std::endl;
            return;
        }
        else
        {
            buffer[bytesRead] = '\0';

            char *output = (char*)malloc(bytesRead + 1);
            if (output == NULL)
            {
                std::cerr << "[MainWindow] malloc failed." << std::endl;
                continue;
            }
            std::strcpy(output, buffer);

            emit outputReceived(originalOutputFd, output, bytesRead);
        }
    }
}
