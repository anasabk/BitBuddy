#include "Switch.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QEvent>
#include <arpa/inet.h>

Switch::Switch(const char *name, const QString &text1, const QString &text2, QWidget *parent) :
    QGroupBox(parent),
    layout(new QHBoxLayout(this)),
    sw(new QWidget(this)),
    swStick(new QWidget(sw)),
    stickHeight(height - 8),
    label1(new QLabel(text1, this)),
    label2(new QLabel(text2, this))
{
    std::strncpy(switchState.name, name, 32);

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

    char styleSheet[300];

    sw->setFixedSize(stickHeight * 2 + 8, height);
    sprintf(styleSheet, "border: 2px solid #e0e0e0; border-radius: %dpx;", height / 2);
    sw->setStyleSheet(styleSheet);

    swStick->setFixedSize(stickHeight, stickHeight);
    sprintf(styleSheet, "background: #e0e0e0; border-radius: %dpx;", stickHeight / 2);
    swStick->setStyleSheet(styleSheet);

    setState(false);
}

int Switch::serverFd;

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
            setState(!switchState.value);
            write(serverFd, &switchState, sizeof(switchState));
        }
    }

    return false;
}

void Switch::setState(bool value)
{
    swStick->move(!value ? 4 : 4 + stickHeight, 4);

    switchState.value = value;
}

#define RASP_IP "127.0.0.1"
#define RASP_PORT 8081

void Switch::connectToServer()
{
    struct sockaddr_in serverAddr;

    // Create socket
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        perror("[MainWindow] socket");
        return;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(RASP_IP);
    serverAddr.sin_port = htons(RASP_PORT);

    // connect to server
    if (::connect(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("[MainWindow] connect");
        return;
    }
}
