#ifndef TELEMETRYVIEWER_H
#define TELEMETRYVIEWER_H

#include <QGroupBox>
#include <thread>
#include <QLabel>

class TelemetryViewer : public QGroupBox
{
public:
    TelemetryViewer(QWidget *parent = nullptr);
    ~TelemetryViewer();

private:
    struct TelemetryData
    {
        float rightDist;
        float leftDist;
        float xAccel;
        float yAccel;
        float zAccel;
        float temp;
        float xRot;
        float yRot;
        float zRot;
    };

    QLabel *distLabel;
    QLabel *accelLabel;
    QLabel *rotLabel;
    QLabel *tempLabel;

    std::array<QLabel *, 9> labels;

    int sockFd = -1;
    std::thread clientThread;
    std::atomic<bool> isClientRunning = true;
    void runClient();
};

#endif // TELEMETRYVIEWER_H
