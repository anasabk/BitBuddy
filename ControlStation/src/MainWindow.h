#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Camera.h"
#include "ObjectDetection.h"
#include "Joystick.h"
#include "Switch.h"
#include "DesktopCam.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

class MainWindow : public QGroupBox
{
    Q_OBJECT

public:
    MainWindow(const MainWindow &) = delete;
    void operator=(const MainWindow &) = delete;

    static MainWindow *get()
    {
        static MainWindow *instance;

        if (instance == nullptr)
        {
            instance = new MainWindow();
            instance->init();
        }

        return instance;
    }

    ~MainWindow();

private:
    MainWindow() : QGroupBox() {}
    void init();

signals:
    void keyPressed(QKeyEvent *event);
    void keyReleased(QKeyEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override { setSizes(); }

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    DesktopCam desktopCam;

    Camera *camera;
    ObjectDetection *objDet;
    Joystick *joystick;

    QPushButton *startMappingBtn;
    QPushButton *stopMappingBtn;
    QPushButton *killMappingBtn;
    QPushButton *startPathfindingBtn;
    QPushButton *stopPathfindingBtn;

    void enableButton(QPushButton *btn);
    void disableButton(QPushButton *btn);

    QLabel *pathfinding;

    pid_t mappingPid = -1;
    pid_t pathfindingPid = -1;

    Q_SLOT void startMapping();
    Q_SLOT void startPathfinding();

    Q_SLOT void stopMapping();
    Q_SLOT void killMapping();
    Q_SLOT void stopPathfinding();

private slots:
    void setSizes();
    void onSwitchStateChanged(Switch::Type type, bool state);
};

#endif // MAINWINDOW_H
