#include "Camera.h"

#include <iostream>
#include <fstream>
#include <thread>

void yoloInit();
void yoloPrintTotalFrames();
cv::Mat yoloGetFrame();

Camera::Camera(QWidget *parent) :
    QLabel(parent),
    capture("http://127.0.0.1:8080"),
    aspectRatio(0.0f)
{
    setScaledContents(true);

//    class_list = load_class_list();

    if (!capture.isOpened())
    {
        std::cerr << "Error opening video file\n";
        return;
    }

//    load_net(net, false);

//    start = std::chrono::high_resolution_clock::now();

//    new std::thread(&Camera::yolo, this);

    startTimer(10);
}

void Camera::timerEvent(QTimerEvent *event)
{
    cv::Mat frame;

    capture.read(frame);

    if (frame.empty())
    {
        std::cout << "End of stream\n";
        return;
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    setPixmap(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));

    float prevAspectRatio = aspectRatio;
    aspectRatio = static_cast<float>(frame.cols) / frame.rows;

    if (prevAspectRatio != aspectRatio)
        emit aspectRatioChanged(aspectRatio);
}

void Camera::yolo()
{
    while (true)
    {
        cv::Mat frame;

        capture.read(frame);
        if (frame.empty())
        {
            std::cout << "End of stream\n";
            return;
        }

        std::vector<Detection> output;
        detect(frame, net, output, class_list);

        frame_count++;
        total_frames++;

        int detections = output.size();

        for (int i = 0; i < detections; ++i)
        {
            auto detection = output[i];
            auto box = detection.box;
            auto classId = detection.class_id;
            const auto color = colors[classId % colors.size()];
            cv::rectangle(frame, box, color, 3);

            cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
            cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        }

        if (frame_count >= 30)
        {

            auto end = std::chrono::high_resolution_clock::now();
            fps = frame_count * 1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            frame_count = 0;
            start = std::chrono::high_resolution_clock::now();
        }

        if (fps > 0)
        {

            std::ostringstream fps_label;
            fps_label << std::fixed << std::setprecision(2);
            fps_label << "FPS: " << fps;
            std::string fps_label_str = fps_label.str();

            cv::putText(frame, fps_label_str.c_str(), cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
        }

        setPixmap(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));

        float prevAspectRatio = aspectRatio;
        aspectRatio = static_cast<float>(frame.cols) / frame.rows;

        if (prevAspectRatio != aspectRatio)
            emit aspectRatioChanged(aspectRatio);
    }
}

std::vector<std::string> Camera::load_class_list()
{
    std::vector<std::string> class_list;
    std::ifstream ifs("config_files/classes.txt");
    std::string line;
    while (std::getline(ifs, line))
    {
        class_list.push_back(line);
    }
    return class_list;
}

void Camera::load_net(cv::dnn::Net &net, bool is_cuda)
{
    auto result = cv::dnn::readNet("config_files/best.onnx");
    if (is_cuda)
    {
        std::cout << "Attempting to use CUDA\n";
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    }
    else
    {
        std::cout << "Running on CPU\n";
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }
    net = result;
}

cv::Mat Camera::format_yolov5(const cv::Mat &source) {
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
    source.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}

void Camera::detect(cv::Mat &image, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &className) {
    cv::Mat blob;

    auto input_image = format_yolov5(image);

    cv::dnn::blobFromImage(input_image, blob, 1./255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
    net.setInput(blob);
    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    float x_factor = input_image.cols / INPUT_WIDTH;
    float y_factor = input_image.rows / INPUT_HEIGHT;

    float *data = (float *)outputs[0].data;

    const int rows = 1880;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i) {
        float confidence = data[4];
        if (confidence >= CONFIDENCE_THRESHOLD) {

            float * classes_scores = data + 5;
            cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double max_class_score;
            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
            if (max_class_score > SCORE_THRESHOLD) {

                confidences.push_back(confidence);

                class_ids.push_back(class_id.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];
                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }

        data += 85;
    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
    for (int i = 0; i < nms_result.size(); i++) {
        int idx = nms_result[i];
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        output.push_back(result);
    }
}
