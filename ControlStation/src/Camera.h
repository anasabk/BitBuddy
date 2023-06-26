#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <QLabel>
#include <thread>

class Camera : public QLabel
{
    Q_OBJECT

public:
    Camera(uint16_t port, bool convertToRGB, QWidget *parent = nullptr);
    ~Camera();

signals:
    void aspectRatioChanged(float aspectRatio);  // Emits signal when aspect ratio changes

private:
    uint16_t port;      // Port that will receive the video stream from DesktopCam.
    bool convertToRGB;  // Convert the video stream from BGR to RGB? There is no need to for the object detection overlay as its video stream will be received as RGB.
    float aspectRatio;  // Aspect ratio of the received video stream. Used for adjusting the camera size on the UI.

    int sockFd = -1;
    std::thread clientThread;
    std::atomic<bool> isClientRunning = true;
    void runClient();   // Runs the client that will receive the video stream.
};

#endif // CAMERA_H
