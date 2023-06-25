#include "DesktopCam.h"

#include <opencv2/opencv.hpp>
#include <thread>

DesktopCam::DesktopCam()
{
    std::thread(&DesktopCam::runServer, this).detach();
}

DesktopCam::~DesktopCam()
{
    if (::close(receiverSockFd) == -1)
        perror("[DesktopCam] close 1");
    if (::close(senderSockFd) == -1)
        perror("[DesktopCam] close 2");
}

#define PORT 8082
#define MAX_BUFFER 65507

int DesktopCam::runServer()
{
    receiverSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiverSockFd == -1)
    {
        perror("[DesktopCam] receiver socket");
        return -1;
    }

    senderSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (senderSockFd == -1)
    {
        perror("[DesktopCam] sender socket");
        return -1;
    }

    struct sockaddr_in desktopAddress{};
    desktopAddress.sin_family = AF_INET;
    desktopAddress.sin_addr.s_addr = INADDR_ANY;
    desktopAddress.sin_port = htons(PORT);

    if (bind(receiverSockFd, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1)
    {
        perror("[DesktopCam] bind");
        return -1;
    }

    while (true) {
        uchar buffer[MAX_BUFFER];

        ssize_t bytesReceived = recvfrom(receiverSockFd, buffer, MAX_BUFFER, 0, NULL, NULL);
        if (bytesReceived == -1)
        {
            perror("[DesktopCam] recvfrom");
            continue;
        }

        for (sockaddr_in &address : getClientAddresses())
        {
            if (sendto(senderSockFd, buffer, bytesReceived, 0, (struct sockaddr *)&address, sizeof(address)) == -1)
                perror("[DesktopCam] sendto");
        }
    }

    return 0;
}

std::vector<sockaddr_in> DesktopCam::getClientAddresses()
{
    std::vector<uint16_t> ports = {8083, 8084};
    std::vector<sockaddr_in> addresses;

    for (uint16_t port : ports)
    {
        sockaddr_in address{};

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr("127.0.0.1");
        address.sin_port = htons(port);

        addresses.push_back(address);
    }

    return addresses;
}
