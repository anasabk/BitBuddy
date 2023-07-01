#ifndef RASPTELEMETRY_H
#define RASPTELEMETRY_H

#include "../constants.h"

#include <iostream>
#include <arpa/inet.h>
#include <thread>

class RaspTelemetry
{
public:
    RaspTelemetry()
    {
        serverThread = std::thread(&RaspTelemetry::runServer, this);
    }

    ~RaspTelemetry()
    {
        std::cout << "[RaspTelemetry] Cleaning up..." << std::endl;

        isRunning.store(false);
        serverThread.join();

        if (sockFd != -1)
        {
            if (close(sockFd) == -1)
                perror("[RaspTelemetry] close");
        }

        std::cout << "[RaspTelemetry] Done." << std::endl;
    }

private:
    int sockFd = -1;
    std::thread serverThread;
    std::atomic<bool> isRunning = true;

    struct TelemetryData
    {
        float rightDist;
        float leftDist;
        float xAccel;
        float yAccel;
        float zAccel;
        float temp;
        float xRot;
        float yRot;
        float zRot;
    };

    void runServer()
    {
        if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("[RaspTelemetry] socket");
            return ;
        }

        struct sockaddr_in desktopAddress{};
        desktopAddress.sin_family = AF_INET;
        desktopAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        desktopAddress.sin_port = htons(constants::telemetryPort);

        while (isRunning.load()) {
            TelemetryData data = {
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand(),
                (float)rand() / rand()
            };

            if (sendto(sockFd, &data, sizeof(data), 0, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1)
                perror("[RaspTelemetry] sendto");

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

#endif // RASPTELEMETRY_H
