#ifndef RASPAXES_H
#define RASPAXES_H

#include "../constants.h"

#include <iostream>
#include <arpa/inet.h>
#include <thread>

class RaspAxes
{
public:
    RaspAxes()
    {
        clientThread = std::thread(&RaspAxes::runClient, this);
    }

    ~RaspAxes()
    {
        std::cout << "[RaspAxes] Cleaning up..." << std::endl;

        isRunning.store(false);
        clientThread.join();

        if (sockFd != -1)
        {
            if (close(sockFd) == -1)
                perror("[RaspAxes] close");
        }

        std::cout << "[RaspAxes] Done." << std::endl;
    }

private:
    int sockFd = -1;
    std::thread clientThread;
    std::atomic<bool> isRunning = true;

    struct Axes
    {
        float x, y;
    };

    void runClient() {
        if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("[RaspAxes] socket");
            return ;
        }

        struct sockaddr_in desktopAddress{};
        desktopAddress.sin_family = AF_INET;
        desktopAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        desktopAddress.sin_port = htons(constants::joystickPort);

        std::cout << "[RaspAxes] Sending address to desktop..." << std::endl;

        for (int i = 0; i < 10; i++)
        {
            if (sendto(sockFd, NULL, 0, 0, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1) {
                perror("[RaspAxes] sendto");
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        struct timeval optval = {0, 100000};
        if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval)) == -1)
            perror("[DesktopCam] setsockopt");

        while (isRunning.load()) {
            Axes axes;

            ssize_t bytesReceived = recvfrom(sockFd, &axes, sizeof(axes), 0, NULL, NULL);
            if (bytesReceived == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    perror("[RaspAxes] recvfrom");

                continue;
            }

//            std::cout << "[RaspAxes] Received axes: " << "x: " << axes.x << ", y: " << axes.y << std::endl;
        }
    }
};

#endif // RASPAXES_H
