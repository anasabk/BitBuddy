#include "MainWindow.h"

#include <signal.h>
#include <QScrollBar>

MainWindow::MainWindow(Console *console) :
    QGroupBox(),
    HBox1(new QGroupBox(this)),
    HBox2(new QGroupBox(this)),
    cameraVBox(new QGroupBox(HBox1)),
    objDetVBox(new QGroupBox(HBox1)),
    console(console),
    joystick(new Joystick(200, HBox2)),
    switchesVBox(new QGroupBox(this)),

    VBoxLayout(new QVBoxLayout(this)),
    HBox1Layout(new QHBoxLayout(HBox1)),
    HBox2Layout(new QHBoxLayout(HBox2)),
    cameraVBoxLayout(new QVBoxLayout(cameraVBox)),
    objDetVBoxLayout(new QVBoxLayout(objDetVBox)),
    switchesVBoxLayout(new QVBoxLayout(switchesVBox)),

    cameraLabel(new QLabel("Camera", cameraVBox)),
    camera(new Camera(8083, true, cameraVBox)),
    objDetLabel(new QLabel("Object Detection", objDetVBox)),
    objDet(new Camera(8085, false, objDetVBox))
{
    connect(camera, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);
    connect(objDet, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);

    setStyleSheet("border: none; background: #303030");

    VBoxLayout->addWidget(HBox1, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    VBoxLayout->addWidget(HBox2, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox1Layout->addWidget(cameraVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    HBox1Layout->addWidget(objDetVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    console->setParent(HBox2);
    HBox2Layout->addWidget(console);
    HBox2Layout->addItem(new QSpacerItem(100, 0));
    HBox2Layout->addWidget(joystick);
    HBox2Layout->addItem(new QSpacerItem(100, 0));
    HBox2Layout->addWidget(switchesVBox);

    cameraVBoxLayout->addWidget(cameraLabel);
    cameraLabel->setStyleSheet("color: #e0e0e0");
    cameraVBoxLayout->addWidget(camera);
    camera->setStyleSheet("border: 2px solid #e0e0e0");

    objDetVBoxLayout->addWidget(objDetLabel);
    objDetLabel->setStyleSheet("color: #e0e0e0");
    objDetVBoxLayout->addWidget(objDet);
    objDet->setStyleSheet("border: 2px solid #e0e0e0");

    switchesVBox->stackUnder(joystick);

    Switch *modeSwitch = new Switch("MODE", "Manual", "Auto", switchesVBox);

    switchesVBoxLayout->addWidget(new Switch("ONOFF", "Off", "On", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(modeSwitch, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("POSE", "Sit", "Stand", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);

    connect(modeSwitch, &Switch::stateChanged, this, &MainWindow::onSwitchStateChanged);

    HBox2Layout->setContentsMargins(0, 40, 200, 40);

    setAttribute(Qt::WA_DeleteOnClose);

    showMaximized();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    setSizes();
}

void MainWindow::setSizes()
{
    float height = size().height() / 2.0f;
    float cameraAspectRatio = (float)camera->pixmap().width() / camera->pixmap().height();
    float objDetAspectRatio = (float)objDet->pixmap().width() / objDet->pixmap().height();

    camera->setFixedSize(height * cameraAspectRatio, height);
    objDet->setFixedSize(height * objDetAspectRatio, height);
}

void MainWindow::onSwitchStateChanged(Switch::SwitchState state)
{
    if (std::strcmp(state.name, "MODE") == 0)
        joystick->setIsDisabled(state.value);
}
