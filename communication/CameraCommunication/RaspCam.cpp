#include <opencv4/opencv2/opencv.hpp>
#include <arpa/inet.h>
#include <thread>

#define DESKTOP_IP "192.168.43.206"
#define DESKTOP_PORT 8082
#define TARGET_FPS 30.0

int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "[RaspCam] Camera not opened." << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd == -1) {
        perror("[RaspCam] socket");
        return -1;
    }

    struct sockaddr_in desktopAddress{};
    desktopAddress.sin_family = AF_INET;
    desktopAddress.sin_addr.s_addr = inet_addr(DESKTOP_IP);
    desktopAddress.sin_port = htons(DESKTOP_PORT);

    while (true) {
        auto start = std::chrono::steady_clock::now();

        cv::Mat frame;
        cap.read(frame);

        if(frame.empty()){
            std::cout << "Frame is empty, not sent" << std::endl;
        
        } else {
            std::vector<uchar> buffer;
            cv::imencode(".jpg", frame, buffer, {cv::IMWRITE_JPEG_QUALITY, 50});

            std::cout << buffer.size() << std::endl;

            if (sendto(sockFd, buffer.data(), buffer.size(), 0, (struct sockaddr *)&desktopAddress, sizeof(desktopAddress)) == -1)
                perror("[RaspCam] sendto");
        }

        auto end = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::nanoseconds((long)(1 / TARGET_FPS * 1e9)) - (end - start));
    }

    return 0;
}


