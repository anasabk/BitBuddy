#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QWidget>

class Joystick : public QWidget
{
public:
    Joystick(int size, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    QWidget *stick;
    float r;
    float sr;
    bool isPressed = false;

    void moveStick(QPoint pos);
};

#endif // JOYSTICK_H
