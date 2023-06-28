#ifndef SWITCH_H
#define SWITCH_H

#include <QGroupBox>
#include <arpa/inet.h>
#include <thread>

class QHBoxLayout;
class QCheckBox;
class QLabel;

class Switch : public QGroupBox
{
    Q_OBJECT

public:
    enum class Type
    {
        OnOff,
        Mode,
        Pose
    };

    Switch(Type type, QWidget *parent = nullptr);
    ~Switch();

    static const std::array<std::array<QString, 3>, 3> texts;
    static std::array<Switch *, 3> createSwitches();

signals:
    void stateChanged(Type type, bool state);

private:
    QWidget *sw;       // Switch
    QWidget *swStick;  // Movable part of the switch.
    const int height;  // Height of the switch.
    const int stickHeight;   // Height of the switch.

    Type type;
    bool state = false;
    static std::array<Switch *, 3> switches;

    void setState(bool state);                         // Sets the switch state.
    bool eventFilter(QObject *object, QEvent *event);  // Handles mouse hover and click events on the switch.

    static struct sockaddr_in clientAddress;  // Robot's address that is received after it connects.
    static socklen_t clientAddressLen;
    static int serverFd;  // Server (switch) socket.
    static std::atomic<int> clientFd;  // Client socket.
    static std::thread manageConnectionThread;
    static std::atomic<bool> isManageConnectionRunning;
    static void startServer();       // Runs the server that will send the changed states to the robot.
    static void manageConnection();  // Accepts connection and checks the status of the current connection.
};

#endif // SWITCH_H
