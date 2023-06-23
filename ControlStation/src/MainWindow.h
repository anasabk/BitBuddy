#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Camera.h"
#include "Joystick.h"
#include "Switch.h"
#include "OutputWatcher.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPlainTextEdit>

class MainWindow : public QGroupBox
{
public:
    MainWindow();

public slots:
    void setSizes();
    void writeOutput(int originalOutputFd, char *output, int n);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QGroupBox *HBox1;
    QGroupBox *HBox2;
    QGroupBox *cameraVBox;
    QGroupBox *objDetVBox;
    QPlainTextEdit *console;
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

    OutputWatcher *stdoutWatcher;
    OutputWatcher *stderrWatcher;
};

#endif // MAINWINDOW_H
