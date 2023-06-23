#ifndef OUTPUTWATCHER_H
#define OUTPUTWATCHER_H

#include <QThread>

class OutputWatcher : public QThread
{
    Q_OBJECT

public:
    OutputWatcher(int outputFd, QObject *parent = nullptr);
    void run() override;

signals:
    void outputReceived(int originalOutputFd, char *output, int n);

private:
    int outputFd;
};

#endif // OUTPUTWATCHER_H
