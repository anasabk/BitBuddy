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
    QGroupBox *HBox1;
    QGroupBox *HBox2;
    QGroupBox *cameraVBox;
    QGroupBox *objDetVBox;
    Console *console;
    Joystick *joystick;
    QGroupBox *switchesVBox;

    QVBoxLayout *VBoxLayout;
    QHBoxLayout *HBox1Layout;
    QHBoxLayout *HBox2Layout;
    QVBoxLayout *cameraVBoxLayout;
    QVBoxLayout *objDetVBoxLayout;
    QVBoxLayout *switchesVBoxLayout;

    QLabel *cameraLabel;
    Camera *camera;
    QLabel *objDetLabel;
    Camera *objDet;

    pid_t objDetPid;
    void startObjDetProcess();

private slots:
    void setSizes();
    void onSwitchStateChanged(Switch::SwitchState state);
};

#endif // MAINWINDOW_H
