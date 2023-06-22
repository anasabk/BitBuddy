#ifndef DESKTOPCAM_H
#define DESKTOPCAM_H

#include <arpa/inet.h>
#include <vector>

class DesktopCam
{
public:
    DesktopCam();
    ~DesktopCam();

private:
    int runServer();
    std::vector<sockaddr_in> getClientAddresses();

    int receiverSockFd;
    int senderSockFd;
};

#endif // DESKTOPCAM_H
