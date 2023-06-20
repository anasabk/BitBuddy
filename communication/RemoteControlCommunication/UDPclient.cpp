#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>
#include <cstdint>

struct Message {
    char type;
    uint32_t id;
    char payload[512];
};

// Mesajın toplam boyutunu aşmaması için payload'u belirli bir boyutta sınırlıyoruz.
static_assert(sizeof(Message) <= 512, "Message size is too large for UDP packet");

int getch() {
    struct termios oldSettings, newSettings;
    tcgetattr(0, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &newSettings);
    int ch = getchar();
    tcsetattr(0, TCSANOW, &oldSettings);
    return ch;
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

    // Send message and receive response from server
    while (true) {
        std::cout << "Press a key (W, A, S, D, Q) or press 'q' to exit: ";
        int ch = getch();

        if (ch == 'q') {
            break;
        } else if (ch == 'w' || ch == 'a' || ch == 's' || ch == 'd') {
            // Create message
            Message message;
            message.type = ch;
            message.id = messageId++;
            strcpy(message.payload, "Test Payload");

            // Send message
            ssize_t bytesSent = sendto(clientSocket, &message, sizeof(message), 0, (struct sockaddr*)&serverAddr, serverAddrSize);
            if (bytesSent < 0) {
                std::cerr << "Error: Could not send message." << std::endl;
                break;
            }

            // Receive response
            Message response;
            ssize_t bytesRead = recvfrom(clientSocket, &response, sizeof(response), 0, NULL, NULL);
            if (bytesRead < 0) {
                std::cerr << "Error: Could not read message." << std::endl;
                break;
            }

            std::cout << "Response from server: " << response.payload << std::endl;
        } else {
            std::cout << "Wrong input. Only 'q', 'w', 'a', 's', 'd' are allowed!" << std::endl;
        }
    }

    // Socket closed
    close(clientSocket);

    return 0;
}
