#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFFER 65536
#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket could not be created" << std::endl;
        return -1;
    }

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Socket could not be bound" << std::endl;
        return -1;
    }

    cv::namedWindow("Raspberry Pi Camera Stream", cv::WINDOW_AUTOSIZE);

    while (true) {
        std::vector<uchar> buffer(MAX_BUFFER);
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        
        int received = recvfrom(sock, &buffer[0], buffer.size(), 0, (struct sockaddr*)&cli_addr, &clilen);
        if (received < 0) {
            std::cerr << "Failed to receive frame" << std::endl;
            continue;
        }

        cv::Mat rawData = cv::Mat(buffer);
        cv::Mat frame = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (frame.empty()) {
            std::cerr << "Decoded frame is empty" << std::endl;
            continue;
        }

        cv::imshow("Raspberry Pi Camera Stream", frame);
        if (cv::waitKey(1) >= 0) break;
    }

    cv::destroyAllWindows();

    return 0;
}
