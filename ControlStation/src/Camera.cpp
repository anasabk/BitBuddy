#include "Camera.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Camera::Camera(QWidget *parent) :
    QLabel(parent)
{
    setScaledContents(true);
    setPixmap(QPixmap::fromImage(QImage(":/camera.png")));

//    sock = socket(AF_INET, SOCK_STREAM, 0);

//    if (sock == -1)
//    {
//        perror("Failed to create socket");
//        exit(EXIT_FAILURE);
//    }

//    struct sockaddr_in serv_addr = {
//        .sin_family = AF_INET,
//        .sin_port = htons(8080)
//    };

//    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) == -1)
//    {
//        perror("Invalid address or address not supported");
//        exit(EXIT_FAILURE);
//    }

//    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
//    {
//        perror("Failed to connect to server");
//        exit(EXIT_FAILURE);
//    }

//    startTimer(20);
}

Camera::~Camera()
{
    ::close(sock);
}

void Camera::timerEvent(QTimerEvent *event)
{
    std::vector<uchar> buf;

    char data[1024] = {0};
    int bytesReceived = recv(sock, data, 1024, 0);

    if (bytesReceived == -1)
    {
        perror("Could not read data from socket");
        exit(EXIT_FAILURE);
    }

    buf.insert(buf.end(), data, data + bytesReceived);

    // Check if there is enough data for a JPEG frame
    auto start = std::search(buf.begin(), buf.end(), std::begin("\xff\xd8"), std::end("\xff\xd8"));
    auto end = std::search(buf.begin(), buf.end(), std::begin("\xff\xd9"), std::end("\xff\xd9"));

    if (start != buf.end() && end != buf.end()) {
        // Extract JPEG frame
        std::vector<uchar> jpgData(start, end + 2);
        buf.erase(buf.begin(), end + 2);

        // Decode JPEG data and display the frame
        cv::Mat frame = imdecode(cv::Mat(jpgData), cv::IMREAD_COLOR);
        setPixmap(QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));
    }
}
