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
    int clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t serverAddrSize = sizeof(serverAddr);

    // Create socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create client socket." << std::endl;
        return 1;
    }

    // Set client address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.43.138");

    // Bind socket
    if (bind(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not bind to client address." << std::endl;
        return 1;
    }

    std::cout << "The client is started. Waiting for server messages..." << std::endl;

    // Set up a delay of 100 ms (10 times per second)
    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 100 * 1000000L; // 100 ms

    // Receive message from server
    while (true) {
        Message message;
        ssize_t bytesRead = recvfrom(clientSocket, &message, sizeof(message), 0, (struct sockaddr*)&clientAddr, &serverAddrSize);
        if (bytesRead < 0) {
            std::cerr << "Error: Message could not be read." << std::endl;
            break;
        }

        std::cout << "Message from server: x=" << message.x << ", y=" << message.y << std::endl;

        // Return response
        Message response;
        response.type = 'R'; // Response type
        response.id = message.id; // Echo back the same message ID
        response.x = message.x;
        response.y = message.y;
        sendto(clientSocket, &response, sizeof(response), 0, (struct sockaddr*)&clientAddr, serverAddrSize);

        // Sleep for the delay period
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, nullptr);
    }

    // Close socket
    close(clientSocket);

    return 0;
}
