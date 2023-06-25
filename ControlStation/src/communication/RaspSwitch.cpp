#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DESKTOP_IP "127.0.0.1"
#define DESKTOP_PORT 8080

struct SwitchState {
    char name[9];
    bool value;
};

int raspSwitch() {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(DESKTOP_PORT);
    serverAddress.sin_addr.s_addr = inet_addr(DESKTOP_IP);

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFd == -1)  {
        perror("[RaspSwitch] socket");
        return -1;
    }

    if(connect(sockFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("[RaspSwitch] connect");
        return -1;
    }

    while(true) {
        SwitchState state;

        if(read(sockFd, &state, sizeof(state)) == -1)
        {
            perror("[RaspSwitch] read");
            continue;
        }
    }
}
