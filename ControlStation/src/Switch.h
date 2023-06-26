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
    // Name is used while sending the state to the robot. text1 and text2 are the names of the states.
    Switch(const char *name, const QString &text1, const QString &text2, QWidget *parent = nullptr);
    ~Switch();

    struct SwitchState
    {
        char name[9];
        bool value;
    };

signals:
    void stateChanged(SwitchState state);

private:
    QWidget *sw;       // Switch
    QWidget *swStick;  // Movable part of the switch.

    SwitchState switchState{};  // State that will be sent to the robot.
    const int height = 24;      // Height of the switch on the UI.
    int stickHeight;            // Height of the switch stick on the UI.

    void setState(bool state);                         // Sets the switch state.
    bool eventFilter(QObject *object, QEvent *event);  // Handles mouse hover and click events on the switch.

    static struct sockaddr_in clientAddress;  // Robot's address that is received after it connects.
    static socklen_t clientAddressLen;
    static int serverFd;  // Server (switch) socket.
    static int clientFd;  // Client socket.
    static std::thread serverThread;
    static std::atomic<bool> isServerRunning;
    static void runServer();                 // Runs the server that will send the changed states to the robot.
    static void sigpipeHandler(int signum);  // Accepts another connection when the current one closes.
    static void acceptClient();              // Accepts connection.
};

#endif // SWITCH_H
