#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <QLabel>

class Camera : public QLabel
{
    Q_OBJECT

public:
    Camera(QWidget *parent = nullptr);

signals:
    void aspectRatioChanged(float aspectRatio);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    float aspectRatio;
    cv::VideoCapture capture;

    const std::vector<cv::Scalar> colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0)};
    const float INPUT_WIDTH = 640.0;
    const float INPUT_HEIGHT = 640.0;
    const float SCORE_THRESHOLD = 0.2;
    const float NMS_THRESHOLD = 0.4;
    const float CONFIDENCE_THRESHOLD = 0.4;

    std::vector<std::string> class_list;

    cv::dnn::Net net;

    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    int frame_count = 0;
    float fps = -1;
    int total_frames = 0;

    struct Detection
    {
        int class_id;
        float confidence;
        cv::Rect box;
    };

    void yolo();
    std::vector<std::string> load_class_list();
    void load_net(cv::dnn::Net &net, bool is_cuda);
    cv::Mat format_yolov5(const cv::Mat &source);
    void detect(cv::Mat &image, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &className);
};

#endif // CAMERA_H
