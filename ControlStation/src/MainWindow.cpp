#include "MainWindow.h"
#include "Camera.h"
#include "Joystick.h"
#include "Switch.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QPlainTextEdit>
#include <signal.h>

MainWindow::MainWindow() :
    QGroupBox(),
    HBox1(new QGroupBox(this)),
    HBox2(new QGroupBox(this)),
    cameraVBox(new QGroupBox(HBox1)),
    objDetVBox(new QGroupBox(HBox1)),
    console(new QPlainTextEdit(HBox2)),
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
//    (new OutputWriter(console, STDOUT_FILENO, this))->start();
//    (new OutputWriter(console, STDERR_FILENO, this))->start();

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

    cameraVBoxLayout->addWidget(cameraLabel);
    cameraLabel->setStyleSheet("color: #e0e0e0");
    cameraVBoxLayout->addWidget(camera);
    camera->setStyleSheet("border: 2px solid #e0e0e0");

    objDetVBoxLayout->addWidget(objDetLabel);
    objDetLabel->setStyleSheet("color: #e0e0e0");
    objDetVBoxLayout->addWidget(objDet);
    objDet->setStyleSheet("border: 2px solid #e0e0e0");

    console->setStyleSheet("border: 1px solid #202020");
    console->setFixedWidth(400);
    console->setReadOnly(true);

    switchesVBox->stackUnder(joystick);

    signal(SIGPIPE, &Switch::sigpipeHandler);
    std::thread(&Switch::runServer).detach();

    switchesVBoxLayout->addWidget(new Switch("ONOFF", "Off", "On", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("MODE", "Manual", "Auto", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);
    switchesVBoxLayout->addWidget(new Switch("POSE", "Sit", "Stand", switchesVBox), 0, Qt::AlignHCenter | Qt::AlignVCenter);

    HBox2Layout->setContentsMargins(0, 40, 200, 40);

    setAttribute(Qt::WA_DeleteOnClose);

    show();
}

MainWindow::~MainWindow()
{
    ::close(STDOUT_FILENO);
    ::close(STDERR_FILENO);
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

#define BUFFER_SIZE 4096

std::mutex MainWindow::OutputWriter::outputMutex;

MainWindow::OutputWriter::OutputWriter(QPlainTextEdit *console, int outputFd, QObject *parent) :
    QThread(parent),
    console(console),
    outputFd(outputFd)
{}

void MainWindow::OutputWriter::run()
{
    int originalOutput;
    int outputFds[2];

    if ((originalOutput = dup(outputFd)) == -1)
    {
        perror("[MainWindow] dup");
        return;
    }

    if (pipe(outputFds) == -1)
    {
        perror("[MainWindow] pipe");
        return;
    }

    if (dup2(outputFds[1], outputFd) == -1)
    {
        perror("[MainWindow] dup2");
        return;
    }

    if (::close(outputFds[1]) == -1)
        perror("[MainWindow] close");

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while (true)
    {
        bytesRead = read(outputFds[0], buffer, BUFFER_SIZE - 1);

        if (bytesRead == -1)
            perror("[MainWindow] read");
        else if (bytesRead == 0)
        {
            std::cout << "[MainWindow] end of outputFd " << outputFd << std::endl;
            return;
        }
        else
        {
            buffer[bytesRead] = '\0';

            outputMutex.lock();
            console->moveCursor(QTextCursor::End);
            console->insertPlainText(buffer);
            console->moveCursor(QTextCursor::End);
            if (write(originalOutput, buffer, bytesRead) == -1)
                perror("[MainWindow] write");
            outputMutex.unlock();
        }
    }
}
