#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

#include "Camera.h"

class ObjectDetection : public Camera
{
    using Camera::Camera;

private:
    inline static const std::vector<cv::Scalar> colors = {
        cv::Scalar(255, 255, 0),
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 255, 255),
        cv::Scalar(255, 0, 0)
    };

    static constexpr float InputWidth = 640.0f;
    static constexpr float InputHeight = 640.0f;
    static constexpr float ScoreThreshold = 0.3f;
    static constexpr float NmsThreshold = 0.7f;
    static constexpr float ConfidenceThreshold = 0.5f;

    struct Detection
    {
        int classId;
        float confidence;
        cv::Rect box;
    };

    decltype(std::chrono::high_resolution_clock::now()) start = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    float fps = -1;

    std::vector<std::string> loadClassNames();
    cv::dnn::Net loadNet(bool useCuda);
    cv::Mat formatYoloV5(const cv::Mat &frame);
    std::vector<Detection> detect(const cv::Mat &frame, cv::dnn::Net &net, const std::vector<std::string> &classNames);
    void processFrame(cv::Mat &frame) override;
};

#endif // OBJECTDETECTION_H
