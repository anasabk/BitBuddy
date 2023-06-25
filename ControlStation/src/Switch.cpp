#include "Switch.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QEvent>
#include <iostream>
#include <thread>
#include <signal.h>

Switch::Switch(const char *name, const QString &text1, const QString &text2, QWidget *parent) :
    QGroupBox(parent),
    layout(new QHBoxLayout(this)),
    sw(new QWidget(this)),
    swStick(new QWidget(sw)),
    stickHeight(height - 8),
    label1(new QLabel(text1, this)),
    label2(new QLabel(text2, this))
{
    static bool init = true;
    if (init)
    {
        signal(SIGPIPE, &Switch::sigpipeHandler);
        std::thread(&Switch::runServer).detach();
        init = false;
    }

    std::strncpy(switchState.name, name, 9);
    switchState.value = false;

    layout->setContentsMargins(0, 0, 0, 0);

    int diff = label1->fontMetrics().size(0, text1).width() - label2->fontMetrics().size(0, text2).width();
    QSpacerItem *spacer = new QSpacerItem(std::abs(diff), 0);

    if (diff < 0)
        layout->addItem(spacer);

    layout->addWidget(label1);
    layout->addWidget(sw);
    layout->addWidget(label2);

    if (diff > 0)
        layout->addItem(spacer);

    if (diff == 0)
        delete spacer;

    label1->setStyleSheet("color: #e0e0e0");
    label2->setStyleSheet("color: #e0e0e0");

    sw->setAttribute(Qt::WA_Hover, true);
    sw->installEventFilter(this);

    sw->setFixedSize(stickHeight * 2 + 8, height);
    sw->setStyleSheet(QString("border: 2px solid #e0e0e0; border-radius: %1px").arg(height / 2));

    swStick->setFixedSize(stickHeight, stickHeight);
    swStick->setStyleSheet(QString("background: #e0e0e0; border-radius: %1px").arg(stickHeight / 2));

    setState(false);
}

Switch::~Switch()
{
    if (serverFd != -1)
    {
        if (::close(serverFd) == -1)
            perror("[Switch] close 1");
        serverFd = -1;
    }

    if (clientFd != -1)
    {
        if (::close(clientFd) == -1)
            perror("[Switch] close 2");
        clientFd = -1;
    }
}

bool Switch::eventFilter(QObject *object, QEvent *event)
{
    if (object == sw)
    {
        if (event->type() == QEvent::HoverEnter)
            setCursor(Qt::PointingHandCursor);
        else if (event->type() == QEvent::HoverLeave)
            setCursor(Qt::ArrowCursor);
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            if (clientFd != -1)
            {
                SwitchState changedState = switchState;
                changedState.value = !switchState.value;

                if (write(clientFd, &changedState, sizeof(changedState)) == -1)
                    perror("[Switch] write");
                else
                {
                    setState(changedState.value);
                    emit stateChanged(changedState);
                    std::cout << "[Switch] Sent changed state: " << switchState.name << " " << switchState.value << std::endl;
                }
            }
            else
                std::cerr << "[Switch] Not connected!" << std::endl;
        }
    }

    return false;
}

void Switch::setState(bool value)
{
    swStick->move(!value ? 4 : 4 + stickHeight, 4);

    switchState.value = value;
}

#define PORT 8080

void Switch::runServer()
{
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[Switch] socket");
        return;
    }

    int optval = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        perror("[Switch] setsockopt");

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("[Switch] bind");
        return;
    }

    if (listen(serverFd, 1) == -1)
    {
        perror("[Switch] listen");
        return;
    }

    std::cout << "[Switch] Bound to port and listening for connection..." << std::endl;

    Switch::acceptClient();
}

void Switch::sigpipeHandler(int signum)
{
    if (::close(clientFd) == -1)
        perror("[Switch] close 3");
    clientFd = -1;
    std::thread(&Switch::acceptClient).detach();
}

void Switch::acceptClient()
{
    if ((clientFd = accept(serverFd, (struct sockaddr *)&Switch::clientAddress, &Switch::clientAddressLen)) == -1)
    {
        perror("[Switch] accept");
        return;
    }

    std::cout << "[Switch] Accepted connection." << std::endl;
}

sockaddr_in Switch::clientAddress;
socklen_t Switch::clientAddressLen = sizeof(Switch::clientAddress);
int Switch::serverFd = -1;
int Switch::clientFd = -1;
