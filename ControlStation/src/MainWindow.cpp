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
    QGroupBox *VBox1 = new QGroupBox(this);
    QGroupBox *VBox2 = new QGroupBox(this);
    QGroupBox *cameraVBox = new QGroupBox(VBox1);
    QGroupBox *objDetVBox = new QGroupBox(VBox1);
    QGroupBox *buttonsHBox = new QGroupBox(VBox2);
    QGroupBox *pathfindingVBox = new QGroupBox(VBox2);
    QGroupBox *HBox1 = new QGroupBox(VBox2);

    joystick = new Joystick(200, HBox1);
    QGroupBox *switchesVBox = new QGroupBox(HBox1);
    TelemetryViewer *telemetryViewer = new TelemetryViewer(HBox1);
    Console *console = new Console(HBox1);

    QHBoxLayout *HBoxLayout = new QHBoxLayout(this);
    QVBoxLayout *VBox1Layout = new QVBoxLayout(VBox1);
    QVBoxLayout *VBox2Layout = new QVBoxLayout(VBox2);
    QVBoxLayout *cameraVBoxLayout = new QVBoxLayout(cameraVBox);
    QVBoxLayout *objDetVBoxLayout = new QVBoxLayout(objDetVBox);
    QHBoxLayout *buttonsHBoxLayout = new QHBoxLayout(buttonsHBox);
    QVBoxLayout *pathfindingVBoxLayout = new QVBoxLayout(pathfindingVBox);
    QHBoxLayout *HBox1Layout = new QHBoxLayout(HBox1);
    QVBoxLayout *switchesVBoxLayout = new QVBoxLayout(switchesVBox);

    QLabel *cameraLabel = new QLabel("Camera", cameraVBox);
    camera = new Camera("Camera", constants::cameraPort, cameraVBox);

    QLabel *objDetLabel = new QLabel("Object Detection", objDetVBox);
    objDet = new ObjectDetection("Object Detection", constants::objDetPort, objDetVBox);

    connect(camera, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);
    connect(objDet, &Camera::aspectRatioChanged, this, &MainWindow::setSizes);

    setStyleSheet("border: none; background: #303030");

    HBoxLayout->addWidget(VBox1, 0, Qt::AlignLeft | Qt::AlignTop);
    HBoxLayout->addWidget(VBox2, 0, Qt::AlignRight);
    HBoxLayout->setContentsMargins(0, 0, 0, 0);

    VBox1Layout->addWidget(cameraVBox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    VBox1Layout->addWidget(objDetVBox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    VBox1Layout->setContentsMargins(0, 0, 0, 0);

    VBox2Layout->addWidget(buttonsHBox, 0, Qt::AlignLeft | Qt::AlignTop);

    startMappingBtn = new QPushButton("Start Mapping", buttonsHBox);
    stopMappingBtn = new QPushButton("Stop Mapping", buttonsHBox);
    killMappingBtn = new QPushButton("Kill Mapping", buttonsHBox);
    startPathfindingBtn = new QPushButton("Start Pathfinding", buttonsHBox);
    stopPathfindingBtn = new QPushButton("Stop Pathfinding", buttonsHBox);

    connect(startMappingBtn, &QPushButton::clicked, this, &MainWindow::startMapping);
    connect(stopMappingBtn, &QPushButton::clicked, this, &MainWindow::stopMapping);
    connect(killMappingBtn, &QPushButton::clicked, this, &MainWindow::killMapping);
    connect(startPathfindingBtn, &QPushButton::clicked, this, &MainWindow::startPathfinding);
    connect(stopPathfindingBtn, &QPushButton::clicked, this, &MainWindow::stopPathfinding);

    startMappingBtn->setCursor(Qt::PointingHandCursor);
    stopMappingBtn->setCursor(Qt::PointingHandCursor);
    killMappingBtn->setCursor(Qt::PointingHandCursor);
    startPathfindingBtn->setCursor(Qt::PointingHandCursor);
    stopPathfindingBtn->setCursor(Qt::PointingHandCursor);

    enableButton(startMappingBtn);
    disableButton(stopMappingBtn);
    disableButton(killMappingBtn);
    enableButton(startPathfindingBtn);
    disableButton(stopPathfindingBtn);

    buttonsHBoxLayout->setSpacing(5);
    buttonsHBoxLayout->addWidget(startMappingBtn);
    buttonsHBoxLayout->addWidget(stopMappingBtn);
    buttonsHBoxLayout->addWidget(killMappingBtn);
    buttonsHBoxLayout->addItem(new QSpacerItem(10, 0));
    buttonsHBoxLayout->addWidget(startPathfindingBtn);
    buttonsHBoxLayout->addWidget(stopPathfindingBtn);

    QLabel *pathfindingLabel = new QLabel("Pathfinding", pathfindingVBox);
    pathfinding = new QLabel(pathfindingVBox);
    pathfinding->setPixmap(QPixmap::fromImage(QImage(":/camera.png")));

    VBox2Layout->addWidget(pathfindingVBox, 0, Qt::AlignLeft | Qt::AlignTop);
    pathfinding->installEventFilter(this);
    VBox2Layout->addWidget(HBox1, 0, Qt::AlignHCenter | Qt::AlignBottom);

    HBox1Layout->addWidget(switchesVBox, Qt::AlignVCenter);
    HBox1Layout->addWidget(joystick, Qt::AlignVCenter);
    HBox1Layout->addWidget(telemetryViewer, Qt::AlignVCenter);
    telemetryViewer->setFixedSize(300, 160);
    HBox1Layout->addWidget(console, Qt::AlignVCenter);
    console->setFixedWidth(500);
    HBox1Layout->setContentsMargins(0, 0, 0, 0);

    cameraVBoxLayout->addWidget(cameraLabel);
    cameraLabel->setStyleSheet(QString("color: %1").arg(constants::white));
    cameraVBoxLayout->addWidget(camera);
    camera->setStyleSheet(QString("border: 2px solid %1").arg(constants::white));

    objDetVBoxLayout->addWidget(objDetLabel);
    objDetLabel->setStyleSheet(QString("color: %1").arg(constants::white));
    objDetVBoxLayout->addWidget(objDet);
    objDet->setStyleSheet(QString("border: 2px solid %1").arg(constants::white));

    pathfindingVBoxLayout->addWidget(pathfindingLabel);
    pathfindingLabel->setStyleSheet(QString("color: %1").arg(constants::white));
    pathfindingVBoxLayout->addWidget(pathfinding);
    pathfinding->setStyleSheet(QString("border: 2px solid %1").arg(constants::white));

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

    if ((pathfindingServerFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
        perror("[MainWindow] socket");

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(constants::pathfindingPort);

    if (bind(pathfindingServerFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
        perror("[MainWindow] bind");

    if (listen(pathfindingServerFd, 1) == -1)
        perror("[MainWindow] listen");

    showMaximized();
}

MainWindow::~MainWindow()
{
    std::cout << "[MainWindow] Cleaning up..." << std::endl;

    killMapping();
    stopPathfinding();

    std::cout << "[MainWindow] Done." << std::endl;
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

void MainWindow::enableButton(QPushButton *btn)
{
    btn->setEnabled(true);
    btn->setStyleSheet(
        QString("color: %1; border: 1px solid %1; background: #404040; padding: 5px").arg(constants::white));
}

void MainWindow::disableButton(QPushButton *btn)
{
    btn->setEnabled(false);
    btn->setStyleSheet(
        QString("color: %1; border: 1px solid %1; background: #282828; padding: 5px").arg(constants::whiteDisabled));
}

void MainWindow::startMapping()
{
    std::cout << "[MainWindow] Starting mapping..." << std::endl;

    mappingPid = fork();

    if (mappingPid == -1)
    {
        perror("[MainWindow] fork 1");
        return;
    }

    if (mappingPid == 0)
    {
        chdir("mapping");
        execlp("./slam", "slam", (char*)NULL);
        perror("[MainWindow] execlp 1");
        _exit(127);
    }
    else
    {
        enableButton(stopMappingBtn);
        enableButton(killMappingBtn);
        disableButton(startMappingBtn);
    }
}

void MainWindow::startPathfinding()
{
    std::cout << "[MainWindow] Starting pathfinding..." << std::endl;

    pathfindingPid = fork();

    if (pathfindingPid == -1)
    {
        perror("[MainWindow] fork 2");
        return;
    }

    if (pathfindingPid == 0)
    {
        chdir("mapping");
        execlp("python3", "pathfinding", "pathfinding.py", (char*)NULL);
        perror("[MainWindow] execlp 2");
        _exit(127);
    }
    else
    {
        enableButton(stopPathfindingBtn);
        disableButton(startPathfindingBtn);
        isPathfindingRendering.store(true);
        pathfindingThread = std::thread(&MainWindow::renderPathfinding, this);
    }
}

void MainWindow::renderPathfinding()
{
    while ((pathfindingClientFd = accept(pathfindingServerFd, nullptr, nullptr)) == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("[MainWindow] accept");
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!isPathfindingRendering.load())
            return;
    }

    while (isPathfindingRendering.load())
    {
        ssize_t bytesRead;
        std::vector<uchar> buffer(constants::maxUdpBuffer);

        if ((bytesRead = read(pathfindingClientFd, &buffer[0], buffer.size())) == -1)
            perror("[MainWindow] read");

        if (bytesRead == 0)
        {
            std::cout << "[MainWindow] Pathfinding connection closed." << std::endl;
            break;
        }

        cv::Mat rawData = cv::Mat(buffer);
        cv::Mat frame = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (frame.empty())
        {
            std::cerr << "[MainWindow] Pathfinding frame is empty." << std::endl;
            continue;
        }

        pathfinding->setPixmap(QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));
    }

    pathfindingClientFd = -1;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && object == pathfinding)
    {
        QMouseEvent *mouseEvent = reinterpret_cast<QMouseEvent *>(event);

        if (pathfindingClientFd != -1)
        {
            int x = mouseEvent->pos().x();
            int y = mouseEvent->pos().y();
            int width = pathfinding->pixmap().width();
            int height = pathfinding->pixmap().height();

            if (write(pathfindingClientFd, &x, sizeof(x)) == -1)
            {
                perror("[MainWindow] write 1");
                return false;
            }

            if (write(pathfindingClientFd, &y, sizeof(y)) == -1)
            {
                perror("[MainWindow] write 2");
                return false;
            }

            if (write(pathfindingClientFd, &width, sizeof(width)) == -1)
            {
                perror("[MainWindow] write 3");
                return false;
            }

            if (write(pathfindingClientFd, &height, sizeof(height)) == -1)
            {
                perror("[MainWindow] write 4");
                return false;
            }
        }
    }

    return false;
}

void MainWindow::stopMapping()
{
    if (mappingPid != -1)
    {
        std::cout << "[MainWindow] Stopping mapping..." << std::endl;

        if (kill(mappingPid, SIGINT) == -1)
            perror("[MainWindow] kill 1");
        //        if (wait(NULL) == -1)
        //            perror("[MainWindow] wait 1");

        //        mappingPid = -1;
    }

    enableButton(startMappingBtn);
}

void MainWindow::killMapping()
{
    if (mappingPid != -1)
    {
        std::cout << "[MainWindow] Killing mapping..." << std::endl;

        if (kill(mappingPid, SIGKILL) == -1)
            perror("[MainWindow] kill 2");
        if (wait(NULL) == -1)
            perror("[MainWindow] wait 2");

        mappingPid = -1;
    }

    disableButton(stopMappingBtn);
    disableButton(killMappingBtn);
    enableButton(startMappingBtn);
}

void MainWindow::stopPathfinding()
{
    if (pathfindingPid != -1)
    {
        std::cout << "[MainWindow] Stopping pathfinding..." << std::endl;

        if (kill(pathfindingPid, SIGKILL) == -1)
            perror("[MainWindow] kill 3");
        if (wait(NULL) == -1)
            perror("[MainWindow] wait 3");

        pathfinding->setPixmap(QPixmap::fromImage(QImage(":/camera.png")));

        isPathfindingRendering.store(false);
        pathfindingThread.join();

        pathfindingPid = -1;
    }

    disableButton(stopPathfindingBtn);
    enableButton(startPathfindingBtn);
}

void MainWindow::setSizes()
{
    //    float height = size().height() / 2.0f;
    //    float cameraAspectRatio = (float)camera->pixmap().width() / camera->pixmap().height();
    //    float objDetAspectRatio = (float)objDet->pixmap().width() / objDet->pixmap().height();

    //    camera->setFixedSize(height * cameraAspectRatio, height);
    //    objDet->setFixedSize(height * objDetAspectRatio, height);
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
