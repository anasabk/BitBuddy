#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
    const int switchPort = 8080;
    const int joystickPort = 8081;
    const int desktopCamPort = 8082;
    const int uiCameraPort = 8083;
    const int objDetport = 8084;
    const int uiObjDetPort = 8085;

    const int maxUdpBuffer = 65507;
    const int targetFps = 30;
    const int joystickSendRate = 10;
}

#endif // CONSTANTS_H
