#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize;
    char buffer[1024];

    // Create socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error: Failed to create server socket." << std::endl;
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not bind to server address." << std::endl;
        return 1;
    }

    std::cout << "The server is started. Waiting for client messages..." << std::endl;

    // Receive message from client
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        clientAddrSize = sizeof(clientAddr);
        int bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (bytesRead < 0) {
            std::cerr << "Error: Message could not be read." << std::endl;
            break;
        }

        std::cout << "Message from client: " << buffer << std::endl;

        // return response
        std::string response = "Message received.";
        sendto(serverSocket, response.c_str(), response.size(), 0, (struct sockaddr*)&clientAddr, clientAddrSize);
    }

    // Close socket
    close(serverSocket);

    return 0;
}
