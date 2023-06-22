#include "Camera.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>

Camera::Camera(QWidget *parent) :
    QLabel(parent),
    aspectRatio(0.0f)
{
    setScaledContents(true);

    std::thread(&Camera::runClient, this).detach();
}

#define MAX_BUFFER 65507
#define PORT 8084

void Camera::runClient()
{
    int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd == -1) {
        perror("[Camera] socket");
        return;
    }

    struct sockaddr_in clientAddress{};
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    clientAddress.sin_port = htons(PORT);

    if (bind(sockFd, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1) {
        perror("[Camera] bind");
        return;
    }

    while (true) {
        std::vector<uchar> buffer(MAX_BUFFER);

        ssize_t bytesReceived = recvfrom(sockFd, &buffer[0], buffer.size(), 0, NULL, NULL);
        if (bytesReceived == -1) {
            perror("[Camera] recvfrom");
            continue;
        }

        cv::Mat rawData = cv::Mat(buffer);
        cv::Mat frame = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (frame.empty()) {
            std::cerr << "[Camera] Decoded frame is empty." << std::endl;
            continue;
        }

        setPixmap(QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));

        float prevAspectRatio = aspectRatio;
        aspectRatio = (float)frame.cols / frame.rows;

        if (prevAspectRatio != aspectRatio)
            emit aspectRatioChanged(aspectRatio);
    }
}
