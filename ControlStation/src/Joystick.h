#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QWidget>

class Joystick : public QWidget
{
public:
    Joystick(int size, QWidget *parent = nullptr);
    ~Joystick();

    void setIsDisabled(bool isDisabled);

protected:
    // Implements joystick movement based on mouse presses.
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    // Implements joystick movement based on key presses.
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    struct Axes
    {
        float x, y;
    };

    QWidget *stick;          // Joystick stick.
    float r;                 // Joystick radius.
    float sr;                // Stick radius.
    bool isPressed = false;  // Is mouse pressed?
    QSet<int> pressedKeys;   // Keys that are currently pressed. Used for implementing joystick movement based on key presses.
    bool isDisabled = false;

    Axes axes;             // Axes that will be sent to the robot.
    std::mutex axesMutex;  // Mutex used when modifying or reading axes. This is needed as the client that sends the axes to the robot runs on a different thread.

    void moveWithPressedKeys();     // Moves stick based on pressed keys.
    void moveStick(QPoint newPos);  // Moves stick relative to joystick.

    int sockFd;        // Socket to send the axes.
    void runClient();  // Run the client that will send the axes.
};

#endif // JOYSTICK_H
