#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <QLabel>
#include <thread>

class Camera : public QLabel
{
    Q_OBJECT

public:
    Camera(std::string name, uint16_t port, QWidget *parent = nullptr);
    virtual ~Camera();

signals:
    void aspectRatioChanged(float aspectRatio);  // Emits signal when aspect ratio changes

protected:
    void cout(std::string message) { std::cout << "[" << name << "] " << message << std::endl; }
    void cerr(std::string message) { std::cerr << "[" << name << "] " << message << std::endl; }
    void perror(std::string message) { ::perror(("[" + name + "] " + message).c_str()); }

private:
    std::string name;          // Name used in messages to the console.
    uint16_t port;             // Port that will receive the video stream from DesktopCam.
    float aspectRatio = 0.0f;  // Aspect ratio of the received video stream. Used for adjusting the camera size on the UI.

    virtual void processFrame(cv::Mat &frame) { cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB); }

    int sockFd = -1;
    std::thread clientThread;
    std::atomic<bool> isClientRunning = true;
    void runClient();  // Runs the client that will receive the video stream.
};

#endif // CAMERA_H
