#ifndef DESKTOPCAM_H
#define DESKTOPCAM_H

#include <arpa/inet.h>
#include <vector>
#include <thread>

class DesktopCam
{
public:
    DesktopCam();
    ~DesktopCam();

private:
    int receiverSockFd = -1;  // Socket to receive the video stream from the robot.
    int senderSockFd = -1;    // Socket to distribute the video stream to local ports.
    std::thread serverThread;
    std::atomic<bool> isServerRunning = true;
    void runServer();
    std::vector<sockaddr_in> getClientAddresses();  // Get addresses of the local ports to send the video stream to.
};

#endif // DESKTOPCAM_H
