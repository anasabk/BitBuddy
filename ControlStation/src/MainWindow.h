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
    ~MainWindow();

    MainWindow(const MainWindow &) = delete;
    void operator=(const MainWindow &) = delete;

    static MainWindow *get(Console *console = nullptr)
    {
        static MainWindow *instance;

        if (instance == nullptr)
        {
            instance = new MainWindow();
            instance->init(console);
        }

        return instance;
    }

signals:
    void keyPressed(QKeyEvent *event);
    void keyReleased(QKeyEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    MainWindow() : QGroupBox() {};
    void init(Console *console);

    Camera *camera;
    Camera *objDet;
    Joystick *joystick;

    pid_t objDetPid;
    void startObjDetProcess();

private slots:
    void setSizes();
    void onSwitchStateChanged(Switch::Type type, bool state);
};

#endif // MAINWINDOW_H
