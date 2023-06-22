#include <iostream>
#include <arpa/inet.h>

#define PORT 8080

struct Axes {
    float x, y;
};

int raspAxes() {
    int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd == -1) {
        perror("[RaspAxes] socket");
        return -1;
    }

    struct sockaddr_in clientAddress{};
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    clientAddress.sin_port = htons(PORT);

    if (bind(sockFd, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1) {
        perror("[RaspAxes] bind");
        return -1;
    }

    while (true) {
        Axes axes;

        ssize_t bytesReceived = recvfrom(sockFd, &axes, sizeof(axes), 0, NULL, NULL);
        if (bytesReceived == -1) {
            perror("[RaspAxes] recvfrom");
            continue;
        }

//        std::cout << "x: " << axes.x << ", y: " << axes.y << std::endl;
    }

    return 0;
}
