#ifndef RASPCAM_H
#define RASPCAM_H

#include "../constants.h"

#include <opencv2/opencv.hpp>
#include <arpa/inet.h>
#include <thread>

class RaspCam
{
public:
    RaspCam()
    {
        serverThread = std::thread(&RaspCam::runServer, this);
    }

    ~RaspCam()
    {
        std::cout << "[RaspCam] Cleaning up..." << std::endl;

        isRunning.store(false);
        serverThread.join();

        if (sockFd != -1)
        {
            if (close(sockFd) == -1)
                perror("[RaspCam] close");
        }

        std::cout << "[RaspCam] Done." << std::endl;
    }

private:
    int sockFd = -1;
    std::thread serverThread;
    std::atomic<bool> isRunning = true;

    void runServer() {
        cv::VideoCapture cap(0);

        if (!cap.isOpened())
        {
            std::cerr << "[RaspCam] Camera not opened." << std::endl;
            return;
        }

        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            perror("[RaspCam] socket");
            return;
        }

        struct sockaddr_in desktopAddress{};
        desktopAddress.sin_family = AF_INET;
        desktopAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        desktopAddress.sin_port = htons(constants::desktopCamPort);

        while (isRunning.load())
        {
            auto start = std::chrono::steady_clock::now();

            cv::Mat frame;
            cap.read(frame);

            if (frame.empty())
                std::cerr << "[RaspCam] Frame is empty." << std::endl;
            else
            {
                std::vector<uchar> buffer;
                cv::imencode(".jpg", frame, buffer, {cv::IMWRITE_JPEG_QUALITY, 50});

                if (sendto(sockFd, buffer.data(), buffer.size(), 0, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1)
                    perror("[RaspCam] sendto");
            }

            auto end = std::chrono::steady_clock::now();

            std::this_thread::sleep_for(std::chrono::nanoseconds((long)(1.0 / constants::targetFps * 1e9)) - (end - start));
        }
    }
};

#endif // RASPCAM_H
