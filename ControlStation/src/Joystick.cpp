#include "Joystick.h"
#include "constants.h"

#include <iostream>
#include <arpa/inet.h>
#include <QMouseEvent>

Joystick::Joystick(int size, QWidget *parent) :
    QWidget(parent),
    stick(new QWidget(parent)),
    r(size / 2.0f),
    sr(size / 6.0f)
{
    setFixedSize(size, size);
    setStyleSheet(QString("background: rgba(0, 0, 0, 0.3); border: 2px solid #e0e0e0; border-radius: %1px").arg((int)r));

    stick->setFixedSize(sr * 2, sr * 2);
    stick->setStyleSheet(QString("background: #e0e0e0; border-radius: %1px").arg((int)sr));

    stick->setAttribute(Qt::WA_TransparentForMouseEvents);

    stackUnder(stick);
    setFocus();
    moveStick(QPoint(r, r));

    serverThread = std::thread(&Joystick::runServer, this);
}

Joystick::~Joystick()
{
    std::cout << "[Joystick] Cleaning up..." << std::endl;

    isServerRunning.store(false);
    serverThread.join();

    if (sockFd != -1)
    {
        if (::close(sockFd) == -1)
            perror("[Joystick] close");
    }

    std::cout << "[Joystick] Done." << std::endl;
}

void Joystick::setIsDisabled(bool isDisabled)
{
    this->isDisabled = isDisabled;
}

void Joystick::mousePressEvent(QMouseEvent *event)
{
    isPressed = true;

    moveStick(event->pos());
}

void Joystick::mouseReleaseEvent(QMouseEvent *event)
{
    isPressed = false;

    moveStick(QPoint(r, r));
}

void Joystick::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressed)
        moveStick(event->pos());
}

void Joystick::moveEvent(QMoveEvent *event)
{
    moveStick(QPoint(r, r));
}

void Joystick::keyPressEvent(QKeyEvent *event)
{
    pressedKeys += event->key();

    moveWithPressedKeys();
}

void Joystick::keyReleaseEvent(QKeyEvent *event)
{
    pressedKeys -= event->key();

    moveWithPressedKeys();
}

void Joystick::moveWithPressedKeys()
{
    QPoint directions(0, 0);

    if (pressedKeys.contains(Qt::Key_D))
        directions.rx() = 1;
    else if (pressedKeys.contains(Qt::Key_A))
        directions.rx() = -1;

    if (pressedKeys.contains(Qt::Key_W))
        directions.ry() = -1;
    else if (pressedKeys.contains(Qt::Key_S))
        directions.ry() = 1;

    moveStick(QPoint(r, r) + directions * r);
}

void Joystick::moveStick(QPoint newPos)
{
    float x = newPos.x() - r;
    float y = -newPos.y() + r;
    float m = x != 0 ? y / x : std::sqrt(std::numeric_limits<float>::max() - 1.0f);

    float cx = std::sqrt(std::pow(r, 2.0f) / (std::pow(m, 2.0f) + 1.0f)) * std::copysign(1.0f, x);
    float cy = m * cx;

    if (std::pow(x, 2.0f) + std::pow(y, 2.0f) > std::pow(r, 2.0f))
    {
        x = cx;
        y = cy;
    }

    newPos.rx() = x + r - sr;
    newPos.ry() = -(y - r) - sr;
    newPos += pos();

    stick->move(newPos);

    axes.store({x / r, y / r});
}

void Joystick::runServer()
{
    if ((sockFd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1) {
        perror("[Joystick] socket");
        return;
    }

    struct sockaddr_in serverAddress{}, raspAddress{};
    socklen_t raspAddressLen = sizeof(raspAddress);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(constants::joystickPort);

    if (bind(sockFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("[Joystick] bind");
        return;
    }

    std::cout << "[Joystick] Bound to port and waiting to receive address from the robot..." << std::endl;

    while (recvfrom(sockFd, NULL, 0, 0, (struct sockaddr *)&raspAddress, &raspAddressLen) == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("[Joystick] recvfrom");
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!isServerRunning.load())
            return;
    }

    std::cout << "[Joystick] Received address from the robot. Starting to send data." << std::endl;

    clearRecvBuffer();

    while (isServerRunning.load())
    {
        auto start = std::chrono::steady_clock::now();

        if (recvfrom(sockFd, NULL, 0, 0, (struct sockaddr *)&raspAddress, &raspAddressLen) == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("[Joystick] recvfrom");
        }
        else
        {
            std::cout << "[Joystick] Received new address from the robot." << std::endl;

            clearRecvBuffer();
        }

        if (!isDisabled)
        {
            Axes axesValue = axes.load();

            if (sendto(sockFd, &axesValue, sizeof(axesValue), 0, (struct sockaddr *)&raspAddress, sizeof(raspAddress)) == -1)
                perror("[Joystick] sendto");
        }

        auto end = std::chrono::steady_clock::now();

        std::this_thread::sleep_for(std::chrono::nanoseconds((long)(1.0 / constants::joystickSendRate * 1e9)) - (end - start));
    }
}

void Joystick::clearRecvBuffer()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (int i = 0; i < 10; i++)
    {
        if (recvfrom(sockFd, NULL, 0, 0, NULL, NULL) == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("[Joystick] recvfrom");
        }
    }
}
