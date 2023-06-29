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
    static constexpr int typeCount = 3;
    inline static const std::array<std::array<QString, 3>, typeCount> texts = {{
        {"OnOff", "Off", "On"},
        {"Mode", "Manual", "Auto"},
        {"Pose", "Sit", "Stand"}
    }};

    Switch(Type type, QWidget *parent = nullptr);
    ~Switch();

    static void createSwitches();
    static std::array<Switch *, typeCount> getSwitches() { return switches; }

    void setEnabled_(bool enabled);

    Type getType() { return type; }
    bool getState() { return state; }

signals:
    void stateChanged(Type type, bool state);

private:
    QWidget *sw;            // Switch
    QWidget *swStick;       // Movable part of the switch.
    const int height;       // Height of the switch.
    const int stickHeight;  // Height of the switch.
    QLabel *label1;         // Left label
    QLabel *label2;         // Right label

    Type type;
    bool state = false;

    void toggle();                                     // Toggles the switch.
    void setState(bool state);                         // Sets the switch state.
    bool eventFilter(QObject *object, QEvent *event);  // Handles mouse hover and click events on the switch.
    Q_SLOT void onKeyPressed(QKeyEvent *event);        // Handles keyboard shortcut press for the switch.

    static std::array<Switch *, typeCount> switches;

    static struct sockaddr_in clientAddress;  // Robot's address that is received after it connects.
    static socklen_t clientAddressLen;
    static int serverFd;               // Server (switch) socket.
    static std::atomic<int> clientFd;  // Client socket.
    static std::thread manageConnectionThread;
    static std::atomic<bool> isManageConnectionRunning;
    static void startServer();       // Runs the server that will send the changed states to the robot.
    static void manageConnection();  // Accepts connection and checks the status of the current connection.
};

#endif // SWITCH_H
