#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdint>
#include <time.h>

struct Message {
    char type;
    uint32_t id;
    float x, y; 
};

// Make sure the message size does not exceed the limit for UDP packets.
static_assert(sizeof(Message) <= 512, "Message size is too large for UDP packet");


int main() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

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

    // Set up a delay of 100 ms (10 times per second)
    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 100 * 1000000L; // 100 ms

    // Receive message from client
    while (true) {
        Message message;
        ssize_t bytesRead = recvfrom(serverSocket, &message, sizeof(message), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (bytesRead < 0) {
            std::cerr << "Error: Message could not be read." << std::endl;
            break;
        }

        std::cout << "Message from client: " << message.payload << std::endl;

        // Return response
        Message response;
        response.type = 'R'; // Response type
        response.id = message.id; // Echo back the same message ID
        strcpy(response.payload, "Message received.");
        sendto(serverSocket, &response, sizeof(response), 0, (struct sockaddr*)&clientAddr, clientAddrSize);

        // Sleep for the delay period
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, nullptr);
    }

    // Close socket
    close(serverSocket);

    return 0;
}
