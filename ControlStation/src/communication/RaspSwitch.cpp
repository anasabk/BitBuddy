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
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DESKTOP_PORT);
    addr.sin_addr.s_addr = inet_addr(DESKTOP_IP);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)  {
        perror("socket creation failed");
        return -1;
    }

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("socket connection failed");
        return -1;
    }

    while(true) {
        SwitchState state;

        if(read(fd, &state, sizeof(state)) == -1)
        {
            perror("read failed");
            continue;
        }
    }
}
