#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Camera.h"
#include "Joystick.h"
#include "Switch.h"
#include "Console.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>

class MainWindow : public QGroupBox
{
    Q_OBJECT

public:
    MainWindow(Console *console);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Camera *camera;
    Camera *objDet;
    Joystick *joystick;

    pid_t objDetPid;
    void startObjDetProcess();

private slots:
    void setSizes();
    void onSwitchStateChanged(Switch::SwitchState state);
};

#endif // MAINWINDOW_H
