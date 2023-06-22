#ifndef SWITCH_H
#define SWITCH_H

#include <QGroupBox>

class QHBoxLayout;
class QCheckBox;
class QLabel;

class Switch : public QGroupBox
{
public:
    Switch(const char *name, const QString &text1, const QString &text2, QWidget *parent = nullptr);

    static void connectToServer();

private:
    struct SwitchState {
        char name[32];
        bool value;
    };

    QHBoxLayout *layout;
    QWidget *sw;
    QWidget *swStick;
    QLabel *label1;
    QLabel *label2;

    SwitchState switchState;
    const int height = 24;
    int stickHeight;

    bool eventFilter(QObject *object, QEvent *event);
    void setState(bool state);

    static int serverFd;
};

#endif // SWITCH_H
