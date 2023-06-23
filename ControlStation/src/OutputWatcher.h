#ifndef OUTPUTWATCHER_H
#define OUTPUTWATCHER_H

#include <QThread>

class OutputWatcher : public QThread
{
    Q_OBJECT

public:
    OutputWatcher(int outputFd, QObject *parent = nullptr);
    void run() override;  // The function that will run when the thread starts. It duplicates the outputFd and emits when it reads something from it.

signals:
    void outputReceived(int originalOutputFd, char *output, int n);  // Signal for when something is read from outputFd.

private:
    int outputFd;  // The file descriptor of the output to watch.
};

#endif // OUTPUTWATCHER_H
