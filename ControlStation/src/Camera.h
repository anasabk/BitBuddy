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

signals:
    void aspectRatioChanged(float aspectRatio);

private:
    uint16_t port;
    bool convertToRGB;
    float aspectRatio;

    void runClient();
};

#endif // CAMERA_H
