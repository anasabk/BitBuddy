#include "TelemetryViewer.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <iostream>
#include <arpa/inet.h>

TelemetryViewer::TelemetryViewer(QWidget *parent) :
    QGroupBox(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    distLabel = new QLabel("Distance: (l: 0.0, r: 0.0) cm", this);
    accelLabel = new QLabel("Acceleration: (x: 0.0, y: 0.0, z: 0.0) m/s²", this);
    rotLabel = new QLabel("Rotation: (x: 0.0, y: 0.0, z: 0.0) °/s", this);
    tempLabel = new QLabel("Temperature: 0.0 °C", this);

    distLabel->setStyleSheet(QString("color: %1").arg(constants::white));
    accelLabel->setStyleSheet(QString("color: %1").arg(constants::white));
    rotLabel->setStyleSheet(QString("color: %1").arg(constants::white));
    tempLabel->setStyleSheet(QString("color: %1").arg(constants::white));

    layout->addWidget(distLabel);
    layout->addWidget(accelLabel);
    layout->addWidget(rotLabel);
    layout->addWidget(tempLabel);

    clientThread = std::thread(&TelemetryViewer::runClient, this);
}

TelemetryViewer::~TelemetryViewer()
{
    std::cout << "[TelemetryViewer] Cleaning up..." << std::endl;

    isClientRunning.store(false);
    clientThread.join();

    if (sockFd != -1)
    {
        if (::close(sockFd) == -1)
            perror("[Switch] close 1");
    }

    std::cout << "[TelemetryViewer] Done." << std::endl;
}

void TelemetryViewer::runClient()
{
    if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("[TelemetryViewer] socket");
        return ;
    }

    struct sockaddr_in clientAddress{};
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    clientAddress.sin_port = htons(constants::telemetryPort);

    if (bind(sockFd, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1)
    {
        perror("[TelemetryViewer] bind");
        return;
    }

    struct timeval optval = {0, 100000};
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval)) == -1)
        perror("[TelemetryViewer] setsockopt");

    while (isClientRunning.load())
    {
        TelemetryData data;

        if (recvfrom(sockFd, &data, sizeof(data), 0, NULL, NULL) == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("[TelemetryViewer] recvfrom");

            continue;
        }

        distLabel->setText(QString("Distance: (l: %1, r: %2) cm")
            .arg(data.leftDist, 0, 'f', 1)
            .arg(data.rightDist, 0, 'f', 1)
        );
        accelLabel->setText(QString("Acceleration: (x: %1, y: %2, z: %3) m/s²")
            .arg(data.xAccel, 0, 'f', 1)
            .arg(data.yAccel, 0, 'f', 1)
            .arg(data.zAccel, 0, 'f', 1)
        );
        rotLabel->setText(QString("Rotation: (x: %1, y: %2, z: %3) °/s")
            .arg(data.xRot, 0, 'f', 1)
            .arg(data.yRot, 0, 'f', 1)
            .arg(data.zRot, 0, 'f', 1)
        );
        tempLabel->setText(QString("Temperature: %1 °C")
            .arg(data.temp, 0, 'f', 1)
        );
    }
}
