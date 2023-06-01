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
#include <arpa/inet.h>

#define CAMERA_PORT 8080
#define CONTROL_PORT 8081

int cameraServer() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Camera not opened" << std::endl;
        return -1;
    }

    // Set resolution to 640x480
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket could not be created" << std::endl;
        return -1;
    }

    struct sockaddr_in serv_addr;
    std::memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(CAMERA_PORT);

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

        struct timespec timeNow;
        clock_gettime(CLOCK_MONOTONIC, &timeNow);

        while (true) {
            cv::Mat frame;
            cap.read(frame);

            std::vector<uchar> buf;
            cv::imencode(".jpg", frame, buf);
            std::string content =
                "--jpgboundary\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length: " + std::to_string(buf.size()) + "\r\n\r\n";
            send(newsockfd, content.c_str(), content.size(), 0);
            send(newsockfd, reinterpret_cast<char*>(buf.data()), buf.size(), 0);

            // Add 10ms to current time
            timeNow.tv_nsec += 3333333L; // 10 ms in nanoseconds

            // Handle overflow
            while (timeNow.tv_nsec >= 1000000000L) {
                timeNow.tv_nsec -= 1000000000L;
                timeNow.tv_sec++;
            }

            // Sleep until the next 10ms point
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
        }
    }

    return 0;
}

int controlServer()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize;
    char buffer[1024];

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error: Failed to create server socket." << std::endl;
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(CONTROL_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // listen server
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not connect to server address." << std::endl;
        return 1;
    }

    listen(serverSocket, 1);

    std::cout << "The server is started. Waiting for client connection..." << std::endl;

    // Accept client connection
    clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket < 0) {
        std::cerr << "Error: Client connection not accepted." << std::endl;
        return 1;
    }

    std::cout << "Client connection successful." << std::endl;

    // Receive message from client
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead < 0) {
            std::cerr << "Error: Message could not be read." << std::endl;
            break;
        } else if (bytesRead == 0) {
            std::cout << "Client connection closed." << std::endl;
            break;
        }

        std::cout << "Message from client: " << buffer << std::endl;

        // return response
        std::string response = "Message received.";
        write(clientSocket, response.c_str(), response.size());

        char *xStr = strtok(buffer, "|");
        char *yStr = strtok(NULL, "|");

        float x = std::stof(xStr);
        float y = std::stof(yStr);

        std::cout << "x: " << x << ", " << "y: " << y << std::endl;
    }

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
