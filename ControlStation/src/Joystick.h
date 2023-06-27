#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QWidget>
#include <thread>

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

    QWidget *stick;           // Joystick stick.
    float r;                  // Joystick radius.
    float sr;                 // Stick radius.
    bool isPressed = false;   // Is mouse pressed?
    QSet<int> pressedKeys;    // Keys that are currently pressed. Used for implementing joystick movement based on key presses.
    bool isDisabled = false;  // Is joystick disabled?

    std::atomic<Axes> axes;  // Axes that will be sent to the robot.

    void moveWithPressedKeys();     // Moves stick based on pressed keys.
    void moveStick(QPoint newPos);  // Moves stick relative to joystick.

    int sockFd = -1;  // Socket to send the axes.
    std::thread serverThread;
    std::atomic<bool> isServerRunning = true;
    void runServer();  // Run the client that will send the axes.
    void clearRecvBuffer();
};

#endif // JOYSTICK_H
