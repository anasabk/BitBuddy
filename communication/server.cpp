#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Camera not opened" << std::endl;
        return -1;
    }

    // Set resolution to 640x480
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket could not be created" << std::endl;
        return -1;
    }

    struct sockaddr_in serv_addr;
    std::memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Socket not connected" << std::endl;
        return -1;
    }

    listen(sockfd, 5);

    while (true) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            std::cerr << "Socket not accepted" << std::endl;
            return -1;
        }

        std::string header = 
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: multipart/x-mixed-replace; boundary=--jpgboundary\r\n\r\n";
        send(newsockfd, header.c_str(), header.size(), 0);

        while (true) {
            cv::Mat frame;
            cap.read(frame);

            // Convert BGR to RGB
            // cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            std::vector<uchar> buf;
            cv::imencode(".jpg", frame, buf);
            std::string content = 
                "--jpgboundary\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length: " + std::to_string(buf.size()) + "\r\n\r\n";
            send(newsockfd, content.c_str(), content.size(), 0);
            send(newsockfd, reinterpret_cast<char*>(buf.data()), buf.size(), 0);

            usleep(10000);  // Sleep time for FPS control
        }
    }

    return 0;
}