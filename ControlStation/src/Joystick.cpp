#include "Joystick.h"

#include <iostream>
#include <arpa/inet.h>
#include <QMouseEvent>
#include <thread>

Joystick::Joystick(int size, QWidget *parent) :
    QWidget(parent),
    stick(new QWidget(parent)),
    r(size / 2.0f),
    sr(size / 6.0f)
{
    char styleSheet[300];

    setFixedSize(size, size);
    sprintf(styleSheet, "background: rgba(0, 0, 0, 0.3); border: 2px solid #e0e0e0; border-radius: %dpx;", static_cast<int>(r));
    setStyleSheet(styleSheet);

    stick->setFixedSize(sr * 2, sr * 2);
    sprintf(styleSheet, "background: #e0e0e0; border-radius: %dpx;", static_cast<int>(sr));
    stick->setStyleSheet(styleSheet);

    stick->setAttribute(Qt::WA_TransparentForMouseEvents);

    stackUnder(stick);
    setFocus();
    moveStick(QPoint(r, r));

    std::thread(&Joystick::runServer, this).detach();
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
    QPoint axes(0, 0);

    if (pressedKeys.contains(Qt::Key_D))
        axes.rx() = 1;
    else if (pressedKeys.contains(Qt::Key_A))
        axes.rx() = -1;

    if (pressedKeys.contains(Qt::Key_W))
        axes.ry() = -1;
    else if (pressedKeys.contains(Qt::Key_S))
        axes.ry() = 1;

    moveStick(QPoint(r, r) + axes * r);
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

    axesMutex.lock();
    axes = {x / r, y / r};
    axesMutex.unlock();
}

#define RASP_IP "127.0.0.1"
#define RASP_PORT 8080
#define SEND_RATE 10

void Joystick::runServer()
{
    int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd == -1) {
        perror("[Joystick] socket");
        return;
    }

    struct sockaddr_in raspAddress{};
    raspAddress.sin_family = AF_INET;
    raspAddress.sin_addr.s_addr = inet_addr(RASP_IP);
    raspAddress.sin_port = htons(RASP_PORT);

    while (true)
    {
        auto start = std::chrono::steady_clock::now();

        axesMutex.lock();
        if (sendto(sockFd, &axes, sizeof(axes), 0, (struct sockaddr *)&raspAddress, sizeof(raspAddress)) == -1)
            perror("[Joystick] sendto");
        axesMutex.unlock();

        auto end = std::chrono::steady_clock::now();

        std::this_thread::sleep_for(std::chrono::nanoseconds((long)(1.0 / SEND_RATE * 1e9)) - (end - start));
    }
}
