#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGroupBox>
#include <QThread>

class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class Camera;
class Map;
class Joystick;
class Switch;
class QPlainTextEdit;

class MainWindow : public QGroupBox
{
public:
    MainWindow();
    ~MainWindow();

public slots:
    void setSizes();

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
    QLabel *camera;
    QLabel *objDetLabel;
    Camera *objDet;

    class OutputWriter : public QThread
    {
    public:
        OutputWriter(QPlainTextEdit *console, int outputFd, QObject *parent = nullptr);
        void run() override;

    private:
        QPlainTextEdit *console;
        int outputFd;
        static std::mutex outputMutex;
    };
};

#endif // MAINWINDOW_H
