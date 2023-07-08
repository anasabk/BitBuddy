#ifndef RASPSWITCH_H
#define RASPSWITCH_H

#include "../constants.h"
#include "../Switch.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <fcntl.h>

class RaspSwitch
{
public:
    RaspSwitch()
    {
        clientThread = std::thread(&RaspSwitch::runClient, this);
    }

    ~RaspSwitch()
    {
        std::cout << "[RaspSwitch] Cleaning up..." << std::endl;

        isRunning.store(false);
        clientThread.join();

        if (sockFd != -1)
        {
            if (close(sockFd) == -1)
                perror("[RaspSwitch] close");
        }

        std::cout << "[RaspSwitch] Done." << std::endl;
    }

private:
    int sockFd = -1;
    std::thread clientThread;
    std::atomic<bool> isRunning = true;

    void runClient()
    {
        struct sockaddr_in desktopAddress;
        desktopAddress.sin_family = AF_INET;
        desktopAddress.sin_port = htons(constants::switchPort);
        desktopAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

        if((sockFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
        {
            perror("[RaspSwitch] socket");
            return;
        }

        while (connect(sockFd, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1)
        {
            if (errno != EINPROGRESS && errno != ECONNREFUSED)
            {
                perror("[RaspSwitch] connect");
                return;
            }

            if (!isRunning.load())
                return;
        }

        int flags = fcntl(sockFd, F_GETFL);
        if (flags == -1)
            perror("[RaspSwitch] fcntl 1");
        if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1)
            perror("[RaspSwitch] fcntl 2");

        while(isRunning.load())
        {
            ssize_t bytesRead;

            struct {
                Switch::Type type;
                bool state;
            } buf;

            while ((bytesRead = read(sockFd, &buf, sizeof(buf))) == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    perror("[RaspSwitch] read");
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (!isRunning.load())
                    return;
            }

            if (bytesRead == 0)
            {
                std::cerr << "[RaspSwitch] Connection closed." << std::endl;
                return;
            }

            std::cout << "[RaspSwitch] Received changed state: "
                      << Switch::texts[(int)buf.type][0].toStdString() << " "
                      << Switch::texts[(int)buf.type][1 + buf.state].toStdString() << std::endl;
        }
    }
};

#endif // RASPSWITCH_H
