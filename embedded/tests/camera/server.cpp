#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bcm2835.h>
#include <opencv2/opencv.hpp>

#define PORT 5000

int main(int argc, char** argv)
{
    // Initialize the bcm2835 library
    if (!bcm2835_init()) {
        printf("Failed to initialize bcm2835 library\n");
        return -1;
    }

    // Open the camera device
    int fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    // Set the camera format
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("Failed to set camera format");
        return -1;
    }

    // Start streaming video
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("Failed to start streaming");
        return -1;
    }

    // Create a socket
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[640 * 480 * 3];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to a local address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        return -1;
    }

    // Accept incoming connections
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        return -1;
    }

    // Main loop to capture frames and send them over the socket
    while (true) {
        // Capture a frame
        int n = read(fd, buffer, sizeof(buffer));
        if (n < 0) {
            perror("Failed to read frame");
            break;
        }

        // Send the frame over the socket
        if (send(new_socket, buffer, sizeof(buffer), 0) < 0) {
            perror("Failed to send frame");
            break;
        }
    }
	// Stop streaming video
	if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
	    perror("Failed to stop streaming");
	    return -1;
	}

	// Close the camera device
	close(fd);

	// Close the socket
	close(new_socket);
	close(server_fd);

	// Release the bcm2835 library
	bcm2835_close();

	return 0;
}