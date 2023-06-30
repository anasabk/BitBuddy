#ifndef OUTPUTWATCHER_H
#define OUTPUTWATCHER_H

#include <QObject>
#include <thread>

class OutputWatcher : public QObject
{
    Q_OBJECT

public:
    OutputWatcher(int outputFd, QObject *parent = nullptr);
    ~OutputWatcher();

    static const OutputWatcher stdoutWatcher;
    static const OutputWatcher stderrWatcher;

signals:
    void outputRead(QString output);  // Signal for when something is read from outputFd.

private:
    int outputFd;  // The file descriptor of the output to watch.
    std::atomic<bool> isRunning = true;
    std::thread thread;
    void run();  // Duplicate outputFd and when something is read from it, write that to originalOutputFd and emit outputRead signal.

    static const int bufferSize = 4096;
};

#endif // OUTPUTWATCHER_H
