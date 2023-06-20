#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <time.h>

struct SwitchState {
    char name[32];
    char value[32];
};

std::atomic_bool running;

// Function to get the current state of the switches
std::array<SwitchState, 3> getSwitchState() {
    // Fill in with code to get the actual state of the switches
    return {{
        {"ONOFF", "ON"},
        {"MODE", "MAN"},
        {"POSE", "SIT"},
    }};
}

void switchPollingThread(int clientSocket) {
    std::array<SwitchState, 3> lastState = getSwitchState();

    while (running) {
        std::array<SwitchState, 3> currentState = getSwitchState();

        struct timespec delay;
        delay.tv_sec = 0;
        delay.tv_nsec = 100 * 1000000L; // 100 ms

        // Compare the current state of the switches with the last state
        for (size_t i = 0; i < currentState.size(); i++) {
            if (strcmp(currentState[i].name, lastState[i].name) != 0 || strcmp(currentState[i].value, lastState[i].value) != 0) {
                // If switch1 state changes to "off", perform some action
                if (strcmp(currentState[i].name, "ONOFF") == 0 && strcmp(currentState[i].value, "OFF") == 0) {
                    std::cout << "ONOFF is off. Performing action..." << std::endl;
                    //
                }

                if (strcmp(currentState[i].name, "POSE") == 0 && strcmp(currentState[i].value, "SIT") == 0) {
                    std::cout << "POSE is sit. Performing action..." << std::endl;
                    //
                }
                // Send the new state to the server
                write(clientSocket, &currentState[i], sizeof(SwitchState));
                // Update the last state
                lastState[i] = currentState[i];
            }
        }

        // Sleep for a short amount of time to reduce CPU usage
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, nullptr);
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create client socket." << std::endl;
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.43.138");

    // connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not connect to server." << std::endl;
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // Start the switch polling thread
    running = true;
    std::thread pollingThread(switchPollingThread, clientSocket);

    // Wait for the user to press enter to stop
    //std::cin.get();
    running = false;
    pollingThread.join();

    // Soket closed
    close(clientSocket);

    return 0;
}
