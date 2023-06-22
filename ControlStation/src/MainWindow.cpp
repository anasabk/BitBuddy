#include "MainWindow.h"
#include "Camera.h"
#include "Map.h"
#include "Joystick.h"
#include "Switch.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QResizeEvent>

MainWindow::MainWindow() :
    QGroupBox(),
    HBox1(new QGroupBox(this)),
    HBox2(new QGroupBox(this)),
    cameraVBox(new QGroupBox(HBox1)),
    mapVBox(new QGroupBox(HBox1)),
    switchesVBox(new QGroupBox(this)),

    VBoxLayout(new QVBoxLayout(this)),
    HBox1Layout(new QHBoxLayout(HBox1)),
    HBox2Layout(new QHBoxLayout(HBox2)),
    cameraVBoxLayout(new QVBoxLayout(cameraVBox)),
    mapVBoxLayout(new QVBoxLayout(mapVBox)),
    switchesVBoxLayout(new QVBoxLayout(switchesVBox)),

    cameraLabel(new QLabel("Camera", cameraVBox)),
    camera(new Camera(cameraVBox)),
    mapLabel(new QLabel("Map", mapVBox)),
    map(new Map(mapVBox)),

    joystick(new Joystick(200, HBox2))
{
    connect(camera, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);

    setStyleSheet("border: none; background: #303030");

    VBoxLayout->addWidget(HBox1, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    VBoxLayout->addWidget(HBox2, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox1Layout->addWidget(cameraVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    HBox1Layout->addWidget(mapVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox2Layout->addWidget(joystick);
    HBox2Layout->addItem(new QSpacerItem(50, 0));
    HBox2Layout->addWidget(switchesVBox);

    cameraVBoxLayout->addWidget(cameraLabel);
    cameraLabel->setStyleSheet("color: #e0e0e0");
    cameraVBoxLayout->addWidget(camera);
    camera->setStyleSheet("border: 2px solid #e0e0e0");

    mapVBoxLayout->addWidget(mapLabel);
    mapLabel->setStyleSheet("color: #e0e0e0");
    mapVBoxLayout->addWidget(map);
    map->setStyleSheet("border: 2px solid #e0e0e0");

    switchesVBox->stackUnder(joystick);

    Switch::connectToServer();

    switchesVBoxLayout->addWidget(new Switch("ONOFF", "Off", "On", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("MODE", "Manual", "Auto", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("POSE", "Stand", "Sit", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox2Layout->setContentsMargins(250, 40, 0, 40);

    setAttribute(Qt::WA_DeleteOnClose);

    show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    setSizes();
//    switchesVBox->move(joystick->pos() + QPoint(300, 0));
//    switchesVBox->resize(200, 120);
}

void MainWindow::setSizes()
{
    float height = size().height() / 2.0f;
    float cameraAspectRatio = static_cast<float>(camera->pixmap().width()) / camera->pixmap().height();
    float mapAspectRatio = static_cast<float>(map->pixmap().width()) / map->pixmap().height();

    camera->setFixedSize(height * cameraAspectRatio, height);
    map->setFixedSize(height * mapAspectRatio, height);
}
