#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <QLabel>
#include <thread>

class Camera : public QLabel
{
    Q_OBJECT

public:
    Camera(QWidget *parent = nullptr);

signals:
    void aspectRatioChanged(float aspectRatio);

private:
    float aspectRatio;

    void runClient();
};

#endif // CAMERA_H
