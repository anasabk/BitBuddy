#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGroupBox>

class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class Camera;
class Map;
class Joystick;

class MainWindow : public QGroupBox
{
public:
    MainWindow();

public slots:
    void setSizes();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QGroupBox *HBox;
    QGroupBox *cameraVBox;
    QGroupBox *mapVBox;

    QVBoxLayout *VBoxLayout;
    QHBoxLayout *HBoxLayout;
    QVBoxLayout *cameraVBoxLayout;
    QVBoxLayout *mapVBoxLayout;

    QLabel *cameraLabel;
    Camera *camera;
    QLabel *mapLabel;
    Map *map;

    Joystick *joystick;
};

#endif // MAINWINDOW_H
