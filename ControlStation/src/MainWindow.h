#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Camera.h"
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
    ~MainWindow();

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

private:
    MainWindow() : QGroupBox() {};
    void init();

signals:
    void keyPressed(QKeyEvent *event);
    void keyReleased(QKeyEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override { setSizes(); };

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Camera *camera;
    Camera *objDet;
    Joystick *joystick;

    DesktopCam desktopCam;

    pid_t objDetPid;
    void startObjDetProcess();

private slots:
    void setSizes();
    void onSwitchStateChanged(Switch::Type type, bool state);
};

#endif // MAINWINDOW_H
