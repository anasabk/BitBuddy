/*
Bu kod, görüntüyü okur, sıkıştırır ve hem UDP üzerinden yayınlar, 
hem de paylaşılan belleğe yazar. Bu paylaşılan bellek, diğer programlar tarafından okunabilir. 
Mutex, paylaşılan belleğin eşzamanlı olarak güncellenmesini ve okunmasını önler, 
böylece race condition oluşmaz.

Bununla birlikte, 
paylaşılan belleğe erişen diğer programlar aynı mutex'i kullanmalı 
ve aynı paylaşılan bellek alanını okumalıdır.
*/


#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#define PORT 8080
#define SHM_NAME "ImageSHM"
#define MUTEX_NAME "ImageMutex"

int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Camera not opened" << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    boost::interprocess::shared_memory_object shm(boost::interprocess::create_only, SHM_NAME, boost::interprocess::read_write);
    boost::interprocess::named_mutex mutex(boost::interprocess::create_only, MUTEX_NAME);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket could not be created" << std::endl;
        return -1;
    }

    struct sockaddr_in serv_addr, cli_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Socket not bound" << std::endl;
        return -1;
    }

    socklen_t clilen = sizeof(cli_addr);
    std::memset(&cli_addr, 0, sizeof(cli_addr));

    while (true) {
        cv::Mat frame;
        cap.read(frame);
        
        std::vector<uchar> buf;
        cv::imencode(".jpg", frame, buf);

        // Resize shared memory and write image data
        {
            boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex);
            shm.truncate(buf.size());
            boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
            std::memcpy(region.get_address(), buf.data(), buf.size());
        }

        cli_addr.sin_family = AF_INET;
        cli_addr.sin_port = htons(PORT);
        cli_addr.sin_addr.s_addr = inet_addr("http://192.168.1.103:8080"); // put the client's IP address here

        sendto(sockfd, reinterpret_cast<char*>(buf.data()), buf.size(), 0, (struct sockaddr*)&cli_addr, clilen);
    }

    return 0;
}
