#include "Joystick.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>
#include <QMouseEvent>
#include <limits>

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

    connectToServer();
}

void Joystick::connectToServer()
{
    struct sockaddr_in serverAddr;
    char message[1024];
    std::string userInput;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create client socket for joystick." << std::endl;
        return;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connect to server
    if (::connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not connect joystick to server." << std::endl;
        return;
    }

    std::cout << "Connected joystick to server." << std::endl;
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

    // Send joystick axes to server

    char message[1024];

    sprintf(message, "%f|%f", x / r, y / r);

    write(clientSocket, message, 1024);

    // Get Response
    memset(message, 0, sizeof(message));
    int bytesRead = read(clientSocket, message, sizeof(message));
    if (bytesRead < 0) {
        std::cerr << "Error: Could not read Response." << std::endl;
        return;
    } else if (bytesRead == 0) {
        std::cout << "The server connection has been closed." << std::endl;
        return;
    }

    std::cout << "Response from server: " << message << std::endl;
}
