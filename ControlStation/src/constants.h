#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
    constexpr int switchPort = 8080;
    constexpr int joystickPort = 8081;
    constexpr int desktopCamPort = 8082;
    constexpr int uiCameraPort = 8083;
    constexpr int objDetPort = 8084;
    constexpr int uiObjDetPort = 8085;
    constexpr int telemetryPort = 8086;

    constexpr int maxUdpBuffer = 65507;
    constexpr int targetFps = 30;
    constexpr int joystickSendRate = 10;

    constexpr char white[] = "#e0e0e0";
    constexpr char whiteDisabled[] = "#808080";

    constexpr char red[] = "#ff3030";

    constexpr char bg[] = "rgba(0, 0, 0, 0.3)";
    constexpr char bgDisabled[] = "rgba(0, 0, 0, 0.15)";
}

#endif // CONSTANTS_H
