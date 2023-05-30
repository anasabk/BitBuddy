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
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create client socket." << std::endl;
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.104");

    // connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not connect to server." << std::endl;
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // Receive message and send to server
    while (true) {
        std::cout << "Press a key (W, A, S, D, Q) or press 'q' to exit: ";
        int ch = getch();

        if (ch == 'q') {
            break;
        } else if (ch == 'w' || ch == 'a' || ch == 's' || ch == 'd') {
            userInput = ch;

            // Send message to server
            write(clientSocket, userInput.c_str(), userInput.size());

            // Get Response
            memset(message, 0, sizeof(message));
            int bytesRead = read(clientSocket, message, sizeof(message));
            if (bytesRead < 0) {
                std::cerr << "Error: Could not read Response." << std::endl;
                break;
            } else if (bytesRead == 0) {
                std::cout << "The server connection has been closed." << std::endl;
                break;
            }

            std::cout << "\nResponse from server: " << message << std::endl;
        }else{
            std::cout << "Wrong input only q w a s d!!!"<<std::endl;
        }
    }

    // Soket closed
    close(clientSocket);

    return 0;
}
