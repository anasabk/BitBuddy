#ifndef OUTPUTWATCHER_H
#define OUTPUTWATCHER_H

#include <QThread>

class OutputWatcher : public QThread
{
    Q_OBJECT

public:
    OutputWatcher(int outputFd, QObject *parent = nullptr);
    void run() override;  // Duplicate outputFd and when something is read from it, write that to originalOutputFd and emit outputRead signal.

signals:
    void outputRead(char *output, int n);  // Signal for when something is read from outputFd.

private:
    int outputFd;  // The file descriptor of the output to watch.
};

#endif // OUTPUTWATCHER_H
