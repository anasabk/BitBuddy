#include "DesktopCam.h"
#include "constants.h"

#include <opencv2/opencv.hpp>

DesktopCam::DesktopCam()
{
    serverThread = std::thread(&DesktopCam::runServer, this);
}

DesktopCam::~DesktopCam()
{
    std::cout << "[DesktopCam] Cleaning up..." << std::endl;

    isServerRunning.store(false);
    serverThread.join();

    if (receiverSockFd != -1)
    {
        if (::close(receiverSockFd) == -1)
            perror("[DesktopCam] close 1");
    }

    if (senderSockFd != -1)
    {
        if (::close(senderSockFd) == -1)
            perror("[DesktopCam] close 2");
    }

    std::cout << "[DesktopCam] Done." << std::endl;
}

void DesktopCam::runServer()
{
    if ((receiverSockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("[DesktopCam] receiver socket");
        return;
    }

    if ((senderSockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("[DesktopCam] sender socket");
        return;
    }

    struct sockaddr_in desktopAddress{};
    desktopAddress.sin_family = AF_INET;
    desktopAddress.sin_addr.s_addr = INADDR_ANY;
    desktopAddress.sin_port = htons(constants::desktopCamPort);

    if (bind(receiverSockFd, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1)
    {
        perror("[DesktopCam] bind");
        return ;
    }

    struct timeval optval = {0, 100000};
    if (setsockopt(receiverSockFd, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval)) == -1)
        perror("[DesktopCam] setsockopt");

    while (isServerRunning.load())
    {
        uchar buffer[constants::maxUdpBuffer];

        ssize_t bytesReceived = recvfrom(receiverSockFd, buffer, constants::maxUdpBuffer, 0, NULL, NULL);
        if (bytesReceived == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("[DesktopCam] recvfrom");

            continue;
        }

        for (sockaddr_in &address : getClientAddresses())
        {
            if (sendto(senderSockFd, buffer, bytesReceived, 0, (struct sockaddr *)&address, sizeof(address)) == -1)
                perror("[DesktopCam] sendto");
        }
    }
}

std::vector<sockaddr_in> DesktopCam::getClientAddresses()
{
    std::vector<uint16_t> ports = {constants::uiCameraPort, constants::objDetPort};
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
