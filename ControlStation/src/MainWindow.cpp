#include "MainWindow.h"
#include "Camera.h"
#include "Map.h"
#include "Joystick.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QResizeEvent>

MainWindow::MainWindow() :
    QGroupBox(),
    HBox(new QGroupBox(this)),
    cameraVBox(new QGroupBox(HBox)),
    mapVBox(new QGroupBox(HBox)),

    VBoxLayout(new QVBoxLayout(this)),
    HBoxLayout(new QHBoxLayout(HBox)),
    cameraVBoxLayout(new QVBoxLayout(cameraVBox)),
    mapVBoxLayout(new QVBoxLayout(mapVBox)),

    cameraLabel(new QLabel("Camera", cameraVBox)),
    camera(new Camera(cameraVBox)),
    mapLabel(new QLabel("Map", mapVBox)),
    map(new Map(mapVBox)),

    joystick(new Joystick(200, this))
{
    VBoxLayout->addWidget(HBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    VBoxLayout->addWidget(joystick, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBoxLayout->addWidget(cameraVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    HBoxLayout->addWidget(mapVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    cameraVBoxLayout->addWidget(cameraLabel);
    cameraLabel->setStyleSheet("color: #e0e0e0");

    cameraVBoxLayout->addWidget(camera);
    camera->setStyleSheet("border: 2px solid #e0e0e0");

    mapVBoxLayout->addWidget(mapLabel);
    mapLabel->setStyleSheet("color: #e0e0e0");

    mapVBoxLayout->addWidget(map);
    map->setStyleSheet("border: 2px solid #e0e0e0");

    setStyleSheet("border: none");

    HBox->stackUnder(joystick);

    setAttribute(Qt::WA_DeleteOnClose);

    show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    float height = event->size().height() / 2.0f;
    float cameraAspectRatio = static_cast<float>(camera->pixmap().width()) / camera->pixmap().height();
    float mapAspectRatio = static_cast<float>(map->pixmap().width()) / map->pixmap().height();

    camera->setFixedSize(height * cameraAspectRatio, height);
    map->setFixedSize(height * mapAspectRatio, height);
}
