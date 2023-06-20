#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct SwitchState {
    char name[32];
    char value[32];
};

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize;
    SwitchState state;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error: Failed to create server socket." << std::endl;
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // listen server
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not connect to server address." << std::endl;
        return 1;
    }

    listen(serverSocket, 1);

    std::cout << "The server is started. Waiting for client connection..." << std::endl;

    // Accept client connection
    clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket < 0) {
        std::cerr << "Error: Client connection not accepted." << std::endl;
        return 1;
    }

    std::cout << "Client connection successful." << std::endl;

    // Receive message from client
    while (true) {
        memset(&state, 0, sizeof(state));
        int bytesRead = read(clientSocket, &state, sizeof(state));
        if (bytesRead < 0) {
            std::cerr << "Error: Message could not be read." << std::endl;
            break;
        } else if (bytesRead == 0) {
            std::cout << "Client connection closed." << std::endl;
            break;
        }

        std::cout << "Message from client: " << state.name << " changed to " << state.value << std::endl;

        // return response
        std::string response = "Message received.";
        write(clientSocket, response.c_str(), response.size());
    }

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
