#include "MainWindow.h"
#include "Console.h"

#include <QScrollBar>
#include <QKeyEvent>
#include <signal.h>
#include <sys/wait.h>

void MainWindow::init()
{
    startObjDetProcess();

    QGroupBox *HBox1 = new QGroupBox(this);
    QGroupBox *HBox2 = new QGroupBox(this);
    QGroupBox *cameraVBox = new QGroupBox(HBox1);
    QGroupBox *objDetVBox = new QGroupBox(HBox1);
    Console *console = new Console(HBox2);
    joystick = new Joystick(200, HBox2);
    QGroupBox *switchesVBox = new QGroupBox(HBox2);

    QVBoxLayout *VBoxLayout = new QVBoxLayout(this);
    QHBoxLayout *HBox1Layout = new QHBoxLayout(HBox1);
    QHBoxLayout *HBox2Layout = new QHBoxLayout(HBox2);
    QVBoxLayout *cameraVBoxLayout = new QVBoxLayout(cameraVBox);
    QVBoxLayout *objDetVBoxLayout = new QVBoxLayout(objDetVBox);
    QVBoxLayout *switchesVBoxLayout = new QVBoxLayout(switchesVBox);

    QLabel *cameraLabel = new QLabel("Camera", cameraVBox);
    camera = new Camera(8083, true, cameraVBox);

    QLabel *objDetLabel = new QLabel("Object Detection", objDetVBox);
    objDet = new Camera(8085, false, objDetVBox);

    connect(camera, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);
    connect(objDet, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);

    setStyleSheet("border: none; background: #303030");

    VBoxLayout->addWidget(HBox1, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    VBoxLayout->addWidget(HBox2, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox1Layout->addWidget(cameraVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    HBox1Layout->addWidget(objDetVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox2Layout->addWidget(console);
    HBox2Layout->addItem(new QSpacerItem(100, 0));
    HBox2Layout->addWidget(joystick);
    HBox2Layout->addItem(new QSpacerItem(100, 0));
    HBox2Layout->addWidget(switchesVBox);
    HBox2Layout->setContentsMargins(0, 40, 200, 40);

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

MainWindow::~MainWindow()
{
    std::cout << "[MainWindow] Cleaning up..." << std::endl;

    if (objDetPid != -1)
    {
        if (kill(objDetPid, SIGKILL) == -1)
            perror("[MainWindow] kill");
        if (wait(NULL) == -1)
            perror("[MainWindow] wait");
    }

    std::cout << "[MainWindow] Done." << std::endl;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    setSizes();
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

void MainWindow::startObjDetProcess()
{
    objDetPid = fork();

    if (objDetPid == -1)
    {
        perror("[MainWindow] fork");
        return;
    }

    if (objDetPid == 0)
    {
        execlp("python3", "python3", "yolo.py", (char*)NULL);
        perror("[MainWindow] execlp");
        _exit(127);
    }
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
