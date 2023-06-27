#ifndef RASPSWITCH_H
#define RASPSWITCH_H

#include "../constants.h"

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

    struct SwitchState
    {
        char name[9];
        bool value;
    };

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

        while(isRunning.load())
        {
            SwitchState state;

            ssize_t bytesRead = read(sockFd, &state, sizeof(state));

            if (bytesRead == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    perror("[RaspSwitch] read");
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else if (bytesRead == 0)
            {
                std::cout << "[RaspSwitch] Connection closed." << std::endl;
                return;
            }
//            else
//                std::cout << "[RaspSwitch] Received changed state: " << state.name << " " << state.value << std::endl;
        }
    }
};

#endif // RASPSWITCH_H
