#include "Joystick.h"
#include "constants.h"
#include "MainWindow.h"

#include <iostream>
#include <arpa/inet.h>
#include <QMouseEvent>

Joystick::Joystick(int size, QWidget *parent) :
    QWidget(parent),
    stick(new QWidget(parent)),
    r(size / 2.0f),
    sr(size / 6.0f)
{
    connect(MainWindow::get(), &MainWindow::keyPressed, this, &Joystick::onKeyPressed);
    connect(MainWindow::get(), &MainWindow::keyReleased, this, &Joystick::onKeyReleased);

    setFixedSize(size, size);
    stick->setFixedSize(sr * 2, sr * 2);
    moveStick(QPoint(r, r));

    stackUnder(stick);
    stick->setAttribute(Qt::WA_TransparentForMouseEvents);

    setEnabled_(true);

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

void Joystick::setEnabled_(bool enabled)
{
    setEnabled(enabled);

    if (!enabled)
    {
        pressedKeys.clear();
        moveStick(QPoint(r, r));
    }

    QString color = enabled ? constants::white : constants::whiteDisabled;
    QString bg = enabled ? constants::bg : constants::bgDisabled;

    setStyleSheet(QString("background: %1; border: 2px solid %2; border-radius: %3px").arg(bg, color).arg((int)r));
    stick->setStyleSheet(QString("background: %1; border-radius: %2px").arg(color).arg((int)sr));
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

void Joystick::onKeyPressed(QKeyEvent *event)
{
    if (isEnabled())
    {
        pressedKeys += event->key();

        moveWithPressedKeys();
    }
}

void Joystick::onKeyReleased(QKeyEvent *event)
{
    if (isEnabled())
    {
        pressedKeys -= event->key();

        moveWithPressedKeys();
    }
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

    std::cout << "[Joystick] Received the address. Starting to send data." << std::endl;

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
            std::cout << "[Joystick] Received new address." << std::endl;

            clearRecvBuffer();
        }

        if (isEnabled())
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
