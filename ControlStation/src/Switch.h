#ifndef SWITCH_H
#define SWITCH_H

#include <QGroupBox>
#include <arpa/inet.h>

class QHBoxLayout;
class QCheckBox;
class QLabel;

class Switch : public QGroupBox
{
public:
    Switch(const char *name, const QString &text1, const QString &text2, QWidget *parent = nullptr);
    ~Switch();

    static void runServer();
    static void sigpipeHandler(int signum);
    static void acceptClient();

private:
    struct SwitchState {
        char name[9];
        bool value;
    };

    QHBoxLayout *layout;
    QWidget *sw;
    QWidget *swStick;
    QLabel *label1;
    QLabel *label2;

    SwitchState switchState{};
    const int height = 24;
    int stickHeight;

    bool eventFilter(QObject *object, QEvent *event);
    void setState(bool state);

    static struct sockaddr_in clientAddr;
    static socklen_t clientAddrLen;
    static int serverFd;
    static int clientFd;
};

#endif // SWITCH_H
