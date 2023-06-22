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
class Switch;

class MainWindow : public QGroupBox
{
public:
    MainWindow();

public slots:
    void setSizes();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QGroupBox *HBox1;
    QGroupBox *HBox2;
    QGroupBox *cameraVBox;
    QGroupBox *mapVBox;
    QGroupBox *switchesVBox;

    QVBoxLayout *VBoxLayout;
    QHBoxLayout *HBox1Layout;
    QHBoxLayout *HBox2Layout;
    QVBoxLayout *cameraVBoxLayout;
    QVBoxLayout *mapVBoxLayout;
    QVBoxLayout *switchesVBoxLayout;

    QLabel *cameraLabel;
    Camera *camera;
    QLabel *mapLabel;
    Map *map;

    Joystick *joystick;
    std::array<Switch *, 3> switches;
};

#endif // MAINWINDOW_H
