#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8081

struct SwitchState {
    char name[32];
    bool value;
};

int raspSwitch() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize;
    SwitchState state;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("[RaspSwitch] socket");
        return -1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // listen server
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("[RaspSwitch] bind");
        return -1;
    }

    if (listen(serverSocket, 1) == -1)
    {
        perror("[RaspSwitch] listen");
        return -1;
    }

    // Accept client connection
    clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == -1) {
        perror("[RaspSwitch] accept");
        return -1;
    }

    // Receive message from client
    while (true) {
        memset(&state, 0, sizeof(state));
        int bytesRead = read(clientSocket, &state, sizeof(state));
        if (bytesRead == -1) {
            perror("[RaspSwitch] read");
            continue;
        } else if (bytesRead == 0) {
            std::cout << "[RaspSwitch] Client connection closed." << std::endl;
            break;
        }

        std::cout << "[RaspSwitch] Received switch state: " << state.name << " " << state.value << std::endl;
    }

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
