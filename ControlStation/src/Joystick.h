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
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    QWidget *stick;
    float r;
    float sr;
    bool isPressed = false;
    QSet<int> pressedKeys;

    void moveWithPressedKeys();
    void moveStick(QPoint newPos);

    int clientSocket;

    void connectToServer();
};

#endif // JOYSTICK_H
