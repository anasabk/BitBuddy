#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    // Set up socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error: could not create socket");
        exit(1);
    }

    // Bind to local address
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error: could not bind to address");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        perror("Error: could not listen for incoming connections");
        exit(1);
    }

    // Accept incoming connection
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sockfd < 0) {
        perror("Error: could not accept incoming connection");
        exit(1);
    }

    // Open video capture device
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        perror("Error: could not open video capture device");
        exit(1);
    }

    // Set video capture device properties
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // Send video frames over socket
    cv::Mat frame;
    int nbytes;
    int bufsize = 640 * 480 * 3;
    char buffer[bufsize];
    while (true) {
        // Capture video frame
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: could not capture frame\n";
            break;
        }

        // Convert video frame to JPEG format
        std::vector<uchar> jpeg_data;
        cv::imencode(".jpg", frame, jpeg_data);

        // Copy JPEG data to buffer
        std::memcpy(buffer, &jpeg_data[0], jpeg_data.size());

        // Send JPEG data over socket
        nbytes = send(client_sockfd, buffer, jpeg_data.size(), 0);
        if (nbytes < 0) {
            perror("Error: could not send data over socket");
            break;
        }

        // Sleep for a short time to limit frame rate
        usleep(10000);
    }

    // Close connection
    close(client_sockfd);

    return 0;
}