#include "Switch.h"
#include "constants.h"
#include "MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QEvent>
#include <QKeyEvent>
#include <iostream>
#include <signal.h>
#include <fcntl.h>

Switch::Switch(Type type, QWidget *parent) :
    QGroupBox(parent),
    height(24),
    stickHeight(height - 8),
    type(type)
{
    connect(MainWindow::get(), &MainWindow::keyPressed, this, &Switch::onKeyPressed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    sw = new QWidget(this);
    swStick = new QWidget(sw);
    label1 = new QLabel(Switch::texts[(int)type][1], this);
    label2 = new QLabel(Switch::texts[(int)type][2], this);

    static bool init = true;
    if (init)
    {
        Switch::startServer();
        init = false;
    }

    layout->setContentsMargins(0, 0, 0, 0);

    int diff = label1->fontMetrics().size(0, label1->text()).width() - label2->fontMetrics().size(0, label2->text()).width();
    auto *spacer = new QSpacerItem(std::abs(diff), 0);

    if (diff < 0)
        layout->addItem(spacer);

    layout->addWidget(label1);
    layout->addWidget(sw);
    layout->addWidget(label2);

    if (diff > 0)
        layout->addItem(spacer);

    if (diff == 0)
        delete spacer;

    sw->setAttribute(Qt::WA_Hover, true);
    sw->installEventFilter(this);

    sw->setFixedSize(stickHeight * 2 + 8, height);
    swStick->setFixedSize(stickHeight, stickHeight);

    setEnabled_(true);
    setState(false);
}

Switch::~Switch()
{
    static bool cleanup = true;
    if (cleanup)
    {
        std::cout << "[Switch] Cleaning up..." << std::endl;

        Switch::isManageConnectionRunning.store(false);
        Switch::manageConnectionThread.join();

        if (Switch::serverFd != -1)
        {
            if (::close(serverFd) == -1)
                perror("[Switch] close 1");
        }

        if (Switch::clientFd != -1)
        {
            if (::close(clientFd) == -1)
                perror("[Switch] close 2");
        }

        std::cout << "[Switch] Done." << std::endl;

        cleanup = false;
    }
}

std::array<Switch *, 3> Switch::switches;
void Switch::createSwitches()
{
    Switch::switches = {
        new Switch(Type::OnOff),
        new Switch(Type::Mode),
        new Switch(Type::Pose)
    };
}

void Switch::setEnabled_(bool enabled)
{
    setEnabled(enabled);

    if (enabled)
        setFocus();

    QString color = enabled ? constants::white : constants::whiteDisabled;
    QString bg = enabled ? constants::bg : constants::bgDisabled;

    label1->setStyleSheet(QString("color: %1").arg(color));
    label2->setStyleSheet(QString("color: %1").arg(color));

    sw->setStyleSheet(QString("background: %1; border: 2px solid %2; border-radius: %3px").arg(bg, color).arg(height / 2));
    swStick->setStyleSheet(QString("background: %1; border-radius: %2px").arg(color).arg(stickHeight / 2));
}

void Switch::toggle()
{
    if (Switch::clientFd.load() != -1)
    {
        bool changedState = !state;

        struct {
            Type type;
            bool state;
        } buf = {type, changedState};

        if (write(Switch::clientFd.load(), &buf, sizeof(buf)) == -1)
        {
            perror("[Switch] write");
            return;
        }

        setState(changedState);
    }
    else
        std::cerr << "[Switch] Not connected!" << std::endl;
}

void Switch::setState(bool state)
{
    swStick->move(!state ? 4 : 4 + stickHeight, 4);

    if (this->state != state)
    {
        this->state = state;
        emit stateChanged(type, state);
    }
}

bool Switch::eventFilter(QObject *object, QEvent *event)
{
    if (object == sw)
    {
        if (event->type() == QEvent::HoverEnter && isEnabled())
            setCursor(Qt::PointingHandCursor);
        else if (event->type() == QEvent::HoverLeave)
            setCursor(Qt::ArrowCursor);
        else if (event->type() == QEvent::MouseButtonRelease && isEnabled())
            toggle();
    }

    return false;
}

void Switch::onKeyPressed(QKeyEvent *event)
{
    if (!event->isAutoRepeat() && event->key() == Qt::Key_1 + (int)type && isEnabled())
        toggle();
}

void Switch::startServer()
{
    if ((Switch::serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    {
        perror("[Switch] socket");
        return;
    }

    int optval = 1;
    if (setsockopt(Switch::serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        perror("[Switch] setsockopt");

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(constants::switchPort);

    if (bind(Switch::serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("[Switch] bind");
        return;
    }

    if (listen(Switch::serverFd, 1) == -1)
    {
        perror("[Switch] listen");
        return;
    }

    std::cout << "[Switch] Bound to port and listening for connection from the robot..." << std::endl;

    Switch::manageConnectionThread = std::thread(&Switch::manageConnection);
}

void Switch::manageConnection()
{
    while (Switch::isManageConnectionRunning.load())
    {
        int ret;
        if ((ret = accept(Switch::serverFd, (struct sockaddr *)&Switch::clientAddress, &Switch::clientAddressLen)) == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("[Switch] accept");
        }
        else
        {
            if (Switch::clientFd.load() != -1)
            {
                if (::close(clientFd) == -1)
                    perror("[Switch] close 3");
            }

            Switch::clientFd.store(ret);

            int flags = fcntl(Switch::clientFd.load(), F_GETFL);
            if (flags == -1)
                perror("[Switch] fcntl 1");
            if (fcntl(Switch::clientFd.load(), F_SETFL, flags | O_NONBLOCK) == -1)
                perror("[Switch] fcntl 2");

            std::cout << "[Switch] Connected." << std::endl;
        }

        if (Switch::clientFd.load() != -1)
        {
            ssize_t bytesRead;
            bool buf;

            if ((bytesRead = read(Switch::clientFd.load(), &buf, sizeof(buf))) == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    if (errno == ECONNRESET)
                        bytesRead = 0;
                    else
                        perror("[Switch] read");
                }
            }

            if (bytesRead == 0)
            {
                Switch::clientFd.store(-1);
                std::cerr << "[Switch] Connection closed." << std::endl;
                for (Switch *sw : Switch::switches)
                    sw->setState(false);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int Switch::serverFd = -1;
std::atomic<int> Switch::clientFd = -1;
sockaddr_in Switch::clientAddress;
socklen_t Switch::clientAddressLen = sizeof(Switch::clientAddress);
std::thread Switch::manageConnectionThread;
std::atomic<bool> Switch::isManageConnectionRunning = true;
