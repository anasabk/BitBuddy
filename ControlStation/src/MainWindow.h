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
    Camera *camera;
    ObjectDetection *objDet;
    Joystick *joystick;

    DesktopCam desktopCam;

    pid_t mappingPid = -1;
    void startMappingProcess();

    QLabel *gridMap;
    pid_t gridMapPid = -1;
    Q_SLOT void startGridMapProcess();

private slots:
    void setSizes();
    void onSwitchStateChanged(Switch::Type type, bool state);
};

#endif // MAINWINDOW_H
