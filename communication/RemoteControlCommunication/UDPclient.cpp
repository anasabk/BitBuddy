#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>

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
    char message[1024];
    std::string userInput;

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
    //serverAddr.sin_addr.s_addr = inet_addr("raspberrypi");

    socklen_t serverAddrSize = sizeof(serverAddr);

    // Send message and receive response from server
    while (true) {
        std::cout << "Press a key (W, A, S, D, Q) or press 'q' to exit: ";
        int ch = getch();

        if (ch == 'q') {
            break;
        } else if (ch == 'w' || ch == 'a' || ch == 's' || ch == 'd') {
            userInput = ch;

            // Send message to server
            sendto(clientSocket, userInput.c_str(), userInput.size(), 0, (struct sockaddr*)&serverAddr, serverAddrSize);

            // Get Response
            memset(message, 0, sizeof(message));
            int bytesRead = recvfrom(clientSocket, message, sizeof(message), 0, NULL, NULL);
            if (bytesRead < 0) {
                std::cerr << "Error: Could not read Response." << std::endl;
                break;
            }

            std::cout << "\nResponse from server: " << message << std::endl;
        }else{
            std::cout << "Wrong input only q w a s d!!!"<<std::endl;
        }
    }

    // Socket closed
    close(clientSocket);

    return 0;
}
