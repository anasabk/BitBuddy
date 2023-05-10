#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace cv;
using namespace std;

int main()
{
    // Create a socket and connect to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "ip address", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    // Receive and display video stream
    vector<uchar> buf;
    while (true) {
        // Receive data from the server
        char data[1024] = {0};
        int bytes_received = recv(sock, data, 1024, 0);
        if (bytes_received <= 0) {
            break;
        }
        buf.insert(buf.end(), data, data + bytes_received);

        // Check if there is enough data for a JPEG frame
        auto start = std::search(buf.begin(), buf.end(), std::begin("\xff\xd8"), std::end("\xff\xd8"));
        auto end = std::search(buf.begin(), buf.end(), std::begin("\xff\xd9"), std::end("\xff\xd9"));
        if (start != buf.end() && end != buf.end()) {
            // Extract JPEG frame
            vector<uchar> jpg_data(start, end + 2);
            buf.erase(buf.begin(), end + 2);

            // Decode JPEG data and display the frame
            Mat frame = imdecode(Mat(jpg_data), IMREAD_COLOR);
            imshow("Video Stream", frame);

            // Exit if the user presses the 'q' key
            if (waitKey(1) & 0xFF == 'q') {
                break;
            }
        }
    }

    // Close the connection and window
    close(sock);
    destroyAllWindows();

    return 0;
}
