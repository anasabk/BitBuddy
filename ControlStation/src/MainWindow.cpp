#include "MainWindow.h"

#include <QScrollBar>
#include <signal.h>
#include <sys/wait.h>

MainWindow::MainWindow(Console *console) :
    QGroupBox()
{
    startObjDetProcess();

    QGroupBox *HBox1 = new QGroupBox(this);
    QGroupBox *HBox2 = new QGroupBox(this);
    QGroupBox *cameraVBox = new QGroupBox(HBox1);
    QGroupBox *objDetVBox = new QGroupBox(HBox1);
    joystick = new Joystick(200, HBox2);
    QGroupBox *switchesVBox = new QGroupBox(this);

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

    console->setParent(HBox2);
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

    std::vector<Switch *> switches = {
        new Switch(Switch::Type::OnOff, switchesVBox),
        new Switch(Switch::Type::Mode, switchesVBox),
        new Switch(Switch::Type::Pose, switchesVBox)
    };

    for (Switch *sw : switches)
    {
        switchesVBoxLayout->addWidget(sw, 0, Qt::AlignHCenter | Qt::AlignVCenter);
        connect(sw, &Switch::stateChanged, this, &MainWindow::onSwitchStateChanged);
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

void MainWindow::onSwitchStateChanged(Switch::State state)
{
    if (state.type == Switch::Type::Mode)
        joystick->setIsDisabled(state.value);
}
