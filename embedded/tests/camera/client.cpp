#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>

#define SERVER_ADDRESS "192.168.0.100"
#define PORT 5000

int main(int argc, char** argv)
{
    // Create a socket
    int client_fd, valread;
    struct sockaddr_in address;
    char buffer[640 * 480 * 3];

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Main loop to receive frames and display them
    while (true) {
        // Receive a frame
        int n = recv(client_fd, buffer, sizeof(buffer), 0);
        if (n < 0) {
            perror("Failed to receive frame");
            break;
        }

        // Convert the received data to an OpenCV Mat object
        cv::Mat frame(480, 640, CV_8UC3, buffer);

        // Display the frame
        cv::imshow("Camera", frame);
        cv::waitKey(1);
    }

    // Close the socket
    close(client_fd);

    return 0;
}