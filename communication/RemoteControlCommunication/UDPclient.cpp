#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>
#include <cstdint>
#include <time.h>
#include <sstream>

struct Message {
    char type;
    uint32_t id;
    float x, y; 
};

// Making sure the total size of the message does not exceed the limit of the UDP packet.
static_assert(sizeof(Message) <= 512, "Message size is too large for UDP packet");

// This function converts the input string into two float values.
void convertData(const std::string& data, float& x, float& y) {
    std::stringstream ss(data);
    std::string item;

    std::getline(ss, item, '|');
    x = std::stof(item);

    std::getline(ss, item, '\n');
    y = std::stof(item);
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrSize = sizeof(serverAddr);

    // Create socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create client socket." << std::endl;
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.43.138");

    uint32_t messageId = 0;

    // Set up a delay of 100 ms (10 times per second)
    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 100 * 1000000L; // 100 ms

    // Send message and receive response from server
    while (true) {
        // Fetch or generate data string.
        std::string data = "1.213 | 2.654"; //replace

        // Convert the string data into float numbers.
        float x, y;
        convertData(data, x, y);

        // Create message
        Message message;
        message.type = 'M'; // Message type
        message.id = messageId++;
        message.x = x;
        message.y = y;

        // Send message
        ssize_t bytesSent = sendto(clientSocket, &message, sizeof(message), 0, (struct sockaddr*)&serverAddr, serverAddrSize);
        if (bytesSent < 0) {
            std::cerr << "Error: Could not send message." << std::endl;
            break;
        }

        // Receive response
        Message response;
        ssize_t bytesRead = recvfrom(clientSocket, &response, sizeof(response), 0, NULL, NULL);
        if (bytesSent < 0) {
            std::cerr << "Error: Could not read message." << std::endl;
            break;
        }

        std::cout << "Response from server: x=" << response.x << ", y=" << response.y << std::endl;

        // Sleep for the delay period
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, nullptr);
    }

    // Socket closed
    close(clientSocket);

    return 0;
}
