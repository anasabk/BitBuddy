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

    std::array<bool, 3> states = {true, false, true};

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

        if (connect(sockFd, (struct sockaddr*)&desktopAddress, sizeof(desktopAddress)) == -1 && errno != EINPROGRESS)
            perror("[RaspSwitch] connect");

        for (bool state : states)
        {
            while (write(sockFd, &state, sizeof(state)) == -1)
            {
                perror("[RaspSwitch] write");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        while(isRunning.load())
        {
            ssize_t bytesRead;
            Switch::Type type;
            bool state;

            while ((bytesRead = read(sockFd, &type, sizeof(type))) == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    perror("[RaspSwitch] read 1");
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (!isRunning.load())
                    return;
            }

            if (bytesRead == 0)
            {
                std::cout << "[RaspSwitch] Connection closed." << std::endl;
                return;
            }

            while ((bytesRead = read(sockFd, &state, sizeof(state))) == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    perror("[RaspSwitch] read 2");
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (!isRunning.load())
                    return;
            }

            if (bytesRead == 0)
            {
                std::cout << "[RaspSwitch] Connection closed." << std::endl;
                return;
            }

            bool buf = true;

            if (write(sockFd, &buf, sizeof(buf)) == -1)
                perror("[RaspSwitch] write");

            std::cout << "[RaspSwitch] Received changed state: "
                      << Switch::texts[(int)type][0].toStdString() << " "
                      << Switch::texts[(int)type][1 + state].toStdString() << std::endl;
        }
    }
};

#endif // RASPSWITCH_H
