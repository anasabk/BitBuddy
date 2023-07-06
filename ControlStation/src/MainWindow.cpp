#include "MainWindow.h"
#include "Console.h"
#include "TelemetryViewer.h"
#include "constants.h"

#include <QScrollBar>
#include <QKeyEvent>
#include <signal.h>
#include <sys/wait.h>

void MainWindow::init()
{
    QGroupBox *HBox1 = new QGroupBox(this);
    QGroupBox *HBox2 = new QGroupBox(this);
    QGroupBox *cameraVBox = new QGroupBox(HBox1);
    QGroupBox *objDetVBox = new QGroupBox(HBox1);
    Console *console = new Console(HBox2);
    joystick = new Joystick(200, HBox2);
    TelemetryViewer *telemetryViewer = new TelemetryViewer(HBox2);
    QGroupBox *switchesVBox = new QGroupBox(HBox2);

    QVBoxLayout *VBoxLayout = new QVBoxLayout(this);
    QHBoxLayout *HBox1Layout = new QHBoxLayout(HBox1);
    QHBoxLayout *HBox2Layout = new QHBoxLayout(HBox2);
    QVBoxLayout *cameraVBoxLayout = new QVBoxLayout(cameraVBox);
    QVBoxLayout *objDetVBoxLayout = new QVBoxLayout(objDetVBox);
    QVBoxLayout *switchesVBoxLayout = new QVBoxLayout(switchesVBox);

    QLabel *cameraLabel = new QLabel("Camera", cameraVBox);
    camera = new Camera("Camera", constants::cameraPort, cameraVBox);

    QLabel *objDetLabel = new QLabel("Object Detection", objDetVBox);
    objDet = new ObjectDetection("Object Detection", constants::objDetPort, objDetVBox);

    connect(camera, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);
    connect(objDet, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);

    setStyleSheet("border: none; background: #303030");

    VBoxLayout->addWidget(HBox1, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    VBoxLayout->addWidget(HBox2, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox1Layout->addWidget(cameraVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    HBox1Layout->addWidget(objDetVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox2Layout->addWidget(console, Qt::AlignVCenter);
    console->setFixedWidth(500);
    HBox2Layout->addItem(new QSpacerItem(100, 0));
    HBox2Layout->addWidget(joystick, Qt::AlignVCenter);
    HBox2Layout->addItem(new QSpacerItem(50, 0));
    HBox2Layout->addWidget(switchesVBox, Qt::AlignVCenter);
    HBox2Layout->addItem(new QSpacerItem(100, 0));
    HBox2Layout->addWidget(telemetryViewer, Qt::AlignVCenter);
    telemetryViewer->setFixedSize(300, 160);
    HBox2Layout->setContentsMargins(30, 40, 0, 40);

    cameraVBoxLayout->addWidget(cameraLabel);
    cameraLabel->setStyleSheet("color: #e0e0e0");
    cameraVBoxLayout->addWidget(camera);
    camera->setStyleSheet("border: 2px solid #e0e0e0");

    objDetVBoxLayout->addWidget(objDetLabel);
    objDetLabel->setStyleSheet("color: #e0e0e0");
    objDetVBoxLayout->addWidget(objDet);
    objDet->setStyleSheet("border: 2px solid #e0e0e0");

    switchesVBox->stackUnder(joystick);

    Switch::createSwitches();
    for (Switch *sw : Switch::getSwitches())
    {
        sw->setParent(switchesVBox);
        switchesVBoxLayout->addWidget(sw, 0, Qt::AlignHCenter | Qt::AlignVCenter);
        connect(sw, &Switch::stateChanged, this, &MainWindow::onSwitchStateChanged);
        onSwitchStateChanged(sw->getType(), false);
    }

    setAttribute(Qt::WA_DeleteOnClose);

    showMaximized();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event);
    QGroupBox::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleased(event);
    QGroupBox::keyReleaseEvent(event);
}

void MainWindow::setSizes()
{
    float height = size().height() / 2.0f;
    float cameraAspectRatio = (float)camera->pixmap().width() / camera->pixmap().height();
    float objDetAspectRatio = (float)objDet->pixmap().width() / objDet->pixmap().height();

    camera->setFixedSize(height * cameraAspectRatio, height);
    objDet->setFixedSize(height * objDetAspectRatio, height);
}

void MainWindow::onSwitchStateChanged(Switch::Type type, bool state)
{
    auto switches = Switch::getSwitches();

    if (type == Switch::Type::OnOff)
    {
        for (int i = 1; i < switches.size(); i++)
            switches[i]->setEnabled_(state);

        if (state && !switches[(int)Switch::Type::Mode]->getState())
            joystick->setEnabled_(true);
        else
            joystick->setEnabled_(false);
    }
    else if (type == Switch::Type::Mode)
    {
        if (!state && switches[(int)Switch::Type::OnOff]->getState())
            joystick->setEnabled_(true);
        else
            joystick->setEnabled_(false);
    }
}
