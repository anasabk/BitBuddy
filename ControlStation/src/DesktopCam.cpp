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

    if (sockFd != -1)
    {
        if (::close(sockFd) == -1)
            perror("[DesktopCam] close 2");
    }

    std::cout << "[DesktopCam] Done." << std::endl;
}

void DesktopCam::runServer()
{
    cv::VideoCapture cap;

    while (!cap.isOpened())
    {
        cap.open(("udp://@:" + std::to_string(constants::desktopCamPort)));

        if (!isServerRunning.load())
            return;
    }

    if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("[DesktopCam] socket");
        return;
    }

    while (isServerRunning.load())
    {
        cv::Mat frame;
        cap.read(frame);

        if (frame.empty())
        {
            std::cerr << "[DesktopCam] Frame is empty." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        cv::rotate(frame, frame, cv::ROTATE_180);

        std::vector<uchar> buffer;
        cv::imencode(".jpg", frame, buffer, {cv::IMWRITE_JPEG_QUALITY, 50});

        for (sockaddr_in &address : getClientAddresses())
        {
            if (sendto(sockFd, buffer.data(), buffer.size(), 0, (struct sockaddr *)&address, sizeof(address)) == -1)
                perror("[DesktopCam] sendto");
        }
    }
}

std::vector<sockaddr_in> DesktopCam::getClientAddresses()
{
    std::vector<uint16_t> ports = {constants::cameraPort, constants::objDetPort};
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
