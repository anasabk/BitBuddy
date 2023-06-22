#include <iostream>
#include <arpa/inet.h>
#include <thread>

#define CLIENT_IP "127.0.0.1"
#define CLIENT_PORT 8081

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
    clientAddress.sin_addr.s_addr = inet_addr(CLIENT_IP);
    clientAddress.sin_port = htons(CLIENT_PORT);

    std::cout << "[RaspAxes] Sending address to client..." << std::endl;

    for (int i = 0; i < 10; i++)
    {
        if (sendto(sockFd, NULL, 0, 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1) {
            perror("[RaspAxes] sendto");
            return -1;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    while (true) {
        Axes axes;

        ssize_t bytesReceived = recvfrom(sockFd, &axes, sizeof(axes), 0, NULL, NULL);
        if (bytesReceived == -1) {
            perror("[RaspAxes] recvfrom");
            continue;
        }
    }

    return 0;
}
