#ifndef SWITCH_H
#define SWITCH_H

#include <QGroupBox>
#include <arpa/inet.h>

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

    struct SwitchState {
        char name[9];
        bool value;
    };

signals:
    void stateChanged(SwitchState state);

private:
    // Layout and widgets for the UI.
    QHBoxLayout *layout;
    QWidget *sw;
    QWidget *swStick;
    QLabel *label1;
    QLabel *label2;

    SwitchState switchState{};  // State that will be sent to the robot.
    const int height = 24;      // Height of the switch on the UI.
    int stickHeight;            // Height of the switch stick on the UI.

    bool eventFilter(QObject *object, QEvent *event);  // Handles mouse hover and click events on the switch.
    void setState(bool state);                         // Sets the switch state.

    // Robot's address that is received after it connects.
    static struct sockaddr_in clientAddress;
    static socklen_t clientAddressLen;

    static int serverFd;  // Server (switch) socket.
    static int clientFd;  // Client socket.

    static void runServer();                 // Runs the server that will send the changed states to the robot.
    static void sigpipeHandler(int signum);  // Accepts another connection when the current one closes.
    static void acceptClient();              // Accepts connection.
};

#endif // SWITCH_H
