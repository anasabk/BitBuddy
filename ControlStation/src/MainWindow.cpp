#include "MainWindow.h"

#include <signal.h>

MainWindow::MainWindow() :
    QGroupBox(),
    HBox1(new QGroupBox(this)),
    HBox2(new QGroupBox(this)),
    cameraVBox(new QGroupBox(HBox1)),
    objDetVBox(new QGroupBox(HBox1)),
    console(new QPlainTextEdit(HBox2)),
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
    objDet(new Camera(8085, false, objDetVBox)),

    stdoutWatcher(new OutputWatcher(STDOUT_FILENO, this)),
    stderrWatcher(new OutputWatcher(STDERR_FILENO, this))
{
    connect(stdoutWatcher, &OutputWatcher::outputReceived, this, &MainWindow::writeOutput);
    connect(stderrWatcher, &OutputWatcher::outputReceived, this, &MainWindow::writeOutput);
    stdoutWatcher->start();
    stderrWatcher->start();

    connect(camera, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);
    connect(objDet, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);

    setStyleSheet("border: none; background: #303030");

    VBoxLayout->addWidget(HBox1, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    VBoxLayout->addWidget(HBox2, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox1Layout->addWidget(cameraVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    HBox1Layout->addWidget(objDetVBox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    joystick = new Joystick(200, HBox2);
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

    console->setStyleSheet("border: 1px solid #202020");
    console->setFixedWidth(500);
    console->setReadOnly(true);

    switchesVBox->stackUnder(joystick);

    signal(SIGPIPE, &Switch::sigpipeHandler);
    std::thread(&Switch::runServer).detach();

    switchesVBoxLayout->addWidget(new Switch("ONOFF", "Off", "On", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("MODE", "Manual", "Auto", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("POSE", "Sit", "Stand", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);

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

void MainWindow::writeOutput(int originalOutputFd, char *output, int n)
{
    console->moveCursor(QTextCursor::End);
    console->insertPlainText(output);
    console->moveCursor(QTextCursor::End);

    if (write(originalOutputFd, output, n) == -1)
        perror("[MainWindow] write");

    free(output);
}
