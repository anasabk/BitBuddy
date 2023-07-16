#include "ObjectDetection.h"
#include "MainWindow.h"

#include <QKeyEvent>
#include <fstream>
#include <opencv2/ccalib/omnidir.hpp>

ObjectDetection::ObjectDetection(std::string name, uint16_t port, QWidget *parent) :
    Camera(name, port, parent)
{
    connect(MainWindow::get(), &MainWindow::keyReleased, this, [&](QKeyEvent *event){
        if (event->key() == Qt::Key_Q)
        {
            ScoreThreshold += 0.01f;
            std::cout << "ScoreThreshold: " << ScoreThreshold << std::endl;
        }
        else if (event->key() == Qt::Key_A)
        {
            ScoreThreshold -= 0.01f;
            std::cout << "ScoreThreshold: " << ScoreThreshold << std::endl;
        }
        else if (event->key() == Qt::Key_W)
        {
            NmsThreshold += 0.01f;
            std::cout << "NmsThreshold: " << NmsThreshold << std::endl;
        }
        else if (event->key() == Qt::Key_S)
        {
            NmsThreshold -= 0.01f;
            std::cout << "NmsThreshold: " << NmsThreshold << std::endl;
        }
        else if (event->key() == Qt::Key_E)
        {
            ConfidenceThreshold += 0.01f;
            std::cout << "ConfidenceThreshold: " << ConfidenceThreshold << std::endl;
        }
        else if (event->key() == Qt::Key_D)
        {
            ConfidenceThreshold -= 0.01f;
            std::cout << "ConfidenceThreshold: " << ConfidenceThreshold << std::endl;
        }
    });
}

void ObjectDetection::processFrame(cv::Mat &frame)
{
    static auto classNames = loadClassNames();
    static auto net = loadNet(false);

    undistortFrame(frame);
    transformFrame(frame);

    auto detections = detect(frame, net, classNames);

    frameCount++;

    for (int i = 0; i < detections.size(); ++i)
    {
        Detection detection = detections[i];
        cv::Rect box = detection.box;
        int classId = detection.classId;
        cv::Scalar color = colors[classId % colors.size()];

        cv::rectangle(frame, box, color, 3);
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
        cv::putText(frame, classNames[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    if (frameCount >= 30)
    {
        auto end = std::chrono::high_resolution_clock::now();
        fps = frameCount * 1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        frameCount = 0;
        start = std::chrono::high_resolution_clock::now();
    }

    if (fps > 0)
    {
        std::ostringstream fpsLabel;

        fpsLabel << std::fixed << std::setprecision(2);
        fpsLabel << "FPS: " << fps;

        cv::putText(frame, fpsLabel.str().c_str(), cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
    }
}

void ObjectDetection::undistortFrame(cv::Mat &frame)
{
    static constexpr bool isOmnidir = false;
    static constexpr double kNewParam = isOmnidir ? 0.5 : 0.9;
    static constexpr int omnidirRectify = cv::omnidir::RECTIFY_PERSPECTIVE;

    static std::string filename = isOmnidir ? "omni" : "fisheye";
    static cv::Mat k = loadMat("calibration/K_" + filename + ".csv", 3, 3);
    static cv::Mat d = loadMat("calibration/d_" + filename + ".csv", isOmnidir ? 1 : 4, isOmnidir ? 4 : 1);
    static cv::Mat xi = isOmnidir ? loadMat("calibration/xi_omni.csv", 1, 1) : cv::Mat();
    static cv::Mat kNew = [&]() {
        cv::Mat kNew = k.clone();
        k.at<double>(0, 0) *= kNewParam;
        k.at<double>(1, 1) *= kNewParam;
        return kNew;
    }();

    if (isOmnidir)
        cv::omnidir::undistortImage(frame, frame, k, d, xi, omnidirRectify, kNew);
    else
        cv::fisheye::undistortImage(frame, frame, k, d, kNew);
}

void ObjectDetection::transformFrame(cv::Mat &frame)
{
    static constexpr float cropHeight = 250;
    static constexpr int perspectiveX = 165;
    static constexpr int perspectiveY = 15;

    frame = frame(cv::Rect(0, frame.rows - cropHeight, frame.cols, cropHeight));

    float width = frame.cols;
    float height = frame.rows;

    float pts1Data[4][2] = {
        {perspectiveX, perspectiveY}, {width - perspectiveX, perspectiveY},
        {0, height}, {width, height}
    };
    float pts2Data[4][2] = {
        {0, 0}, {width, 0},
        {0, height}, {width, height}
    };

    cv::Mat pts1 = cv::Mat(4, 2, CV_32FC1, pts1Data);
    cv::Mat pts2 = cv::Mat(4, 2, CV_32FC1, pts2Data);

    cv::Mat mat = cv::getPerspectiveTransform(pts1, pts2);
    cv::warpPerspective(frame, frame, mat, cv::Size(frame.cols, frame.rows));
}

std::vector<ObjectDetection::Detection>
ObjectDetection::detect(const cv::Mat &frame, cv::dnn::Net &net, const std::vector<std::string> &classNames)
{
    cv::Mat blob;
    cv::Mat inputFrame = formatYoloV5(frame);
    std::vector<cv::Mat> outputs;

    cv::dnn::blobFromImage(inputFrame, blob, 1.0 / 255.0, cv::Size(InputWidth, InputHeight), cv::Scalar(), true, false);
    net.setInput(blob);
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    float xFactor = inputFrame.cols / InputWidth;
    float yFactor = inputFrame.rows / InputHeight;

    float *data = (float *)outputs[0].data;
    int rows = outputs[0].size[1];
    int dimensions = outputs[0].size[2];

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i) {
        float confidence = data[4];

        if (confidence >= ConfidenceThreshold)
        {
            cv::Mat classScores(1, classNames.size(), CV_32FC1, data + 5);
            double maxClassScore;
            cv::Point classId;

            cv::minMaxLoc(classScores, 0, &maxClassScore, 0, &classId);

            if (maxClassScore >= ScoreThreshold)
            {
                confidences.push_back(confidence);
                classIds.push_back(classId.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];

                int left = (x - 0.5 * w) * xFactor;
                int top = (y - 0.5 * h) * yFactor;
                int width = w * xFactor;
                int height = h * yFactor;

                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }

        data += dimensions;
    }

    std::vector<int> nmsIndices;
    std::vector<Detection> detections;

    cv::dnn::NMSBoxes(boxes, confidences, ScoreThreshold, NmsThreshold, nmsIndices);

    for (int idx : nmsIndices)
    {
        Detection detection;
        detection.classId = classIds[idx];
        detection.confidence = confidences[idx];
        detection.box = boxes[idx];

        detections.push_back(detection);
    }

    return detections;
}

cv::Mat ObjectDetection::formatYoloV5(const cv::Mat &frame)
{
    int cols = frame.cols;
    int rows = frame.rows;
    int max = std::max(cols, rows);

    cv::Mat formattedFrame = cv::Mat::zeros(max, max, CV_8UC3);
    frame.copyTo(formattedFrame(cv::Rect(0, 0, cols, rows)));

    return formattedFrame;
}

std::vector<std::string> ObjectDetection::loadClassNames()
{
    std::vector<std::string> classNames;
    std::ifstream ifs("yolo_config/classes.txt");
    std::string line;

    while (getline(ifs, line))
        classNames.push_back(line);

    return classNames;
}

cv::dnn::Net ObjectDetection::loadNet(bool useCuda)
{
    auto net = cv::dnn::readNet("yolo_config/bestv2.onnx");

    if (useCuda)
    {
        cout("Attempting to use CUDA...");
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }
    else
    {
        cout("Running on CPU.");
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    return net;
}

cv::Mat ObjectDetection::loadMat(std::string filename, int rows, int cols)
{
    cv::Mat out = cv::Mat::zeros(rows, cols, CV_64FC1);

    std::ifstream ifs(filename);
    std::string rowStr;
    std::string valStr;

    for (int row = 0; row < rows; row++)
    {
        std::getline(ifs, rowStr);
        std::stringstream rowSs(rowStr);

        for (int col = 0; col < cols; col++)
        {
            std::getline(rowSs, valStr, ',');
            out.at<double>(row, col) = std::stod(valStr);
        }
    }

    return out;
}
