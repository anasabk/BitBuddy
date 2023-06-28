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

    struct State
    {
        Type type;
        bool value;
    };

    Switch(Type type, QWidget *parent = nullptr);
    ~Switch();

    static std::array<std::array<QString, 3>, 3> texts;

signals:
    void stateChanged(State state);

private:
    QWidget *sw;       // Switch
    QWidget *swStick;  // Movable part of the switch.

    State state;                // Switch state
    const int height;           // Height of the switch on the UI.
    int stickHeight;            // Height of the switch stick on the UI.

    void setState(bool state);                         // Sets the switch state.
    bool eventFilter(QObject *object, QEvent *event);  // Handles mouse hover and click events on the switch.

    static struct sockaddr_in clientAddress;  // Robot's address that is received after it connects.
    static socklen_t clientAddressLen;
    static int serverFd;  // Server (switch) socket.
    static std::atomic<int> clientFd;  // Client socket.
    static std::thread acceptThread;
    static std::atomic<bool> isAcceptRunning;
    static void startServer();                 // Runs the server that will send the changed states to the robot.
    static void acceptClient();                // Accepts connection.
};

#endif // SWITCH_H
