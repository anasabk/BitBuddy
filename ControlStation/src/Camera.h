#ifndef CAMERA_H
#define CAMERA_H

#include <QLabel>

class Camera : public QLabel
{
public:
    Camera(QWidget *parent = nullptr);
    ~Camera();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    int sock;
};

#endif // CAMERA_H
