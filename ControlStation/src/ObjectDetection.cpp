#include "ObjectDetection.h"

#include <fstream>

std::vector<std::string> ObjectDetection::loadClassNames()
{
    std::vector<std::string> classNames;
    std::ifstream ifs("config_files/classes.txt");
    std::string line;

    while (getline(ifs, line))
        classNames.push_back(line);

    return classNames;
}

cv::dnn::Net ObjectDetection::loadNet(bool useCuda)
{
    auto net = cv::dnn::readNet("config_files/bestv2.onnx");

    if (useCuda)
    {
        cout("Attempting to use CUDA...");
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    }
    else
    {
        cout("Running on CPU.");
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    return net;
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

void ObjectDetection::processFrame(cv::Mat &frame)
{
    static auto classNames = loadClassNames();
    static auto net = loadNet(false);

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
