#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Raspberry Pi'nin IP adresini ve portunu belirtin
    cv::VideoCapture cap("udp://@:8083"); 

    if (!cap.isOpened()) {
        std::cerr << "Failed to open video stream" << std::endl;
        return -1;
    }

    cv::namedWindow("Raspberry Pi camera", cv::WINDOW_AUTOSIZE);

    while (true) {
        cv::Mat frame;
        cap.read(frame);
        // cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // cv::resize(frame, frame, cv::Size(640, 480));
        cv::imshow("Raspberry Pi camera", frame);

        if (cv::waitKey(1) >= 0) break;
    }

    return 0;
}