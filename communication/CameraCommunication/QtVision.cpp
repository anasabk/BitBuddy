#include <QSharedMemory>
#include <QMutex>
#include <QBuffer>
#include <QDataStream>
#include <opencv2/opencv.hpp>

#define SHM_NAME "ImageSHM"
#define MUTEX_NAME "ImageMutex"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSharedMemory sharedMemory(SHM_NAME);
    QMutex mutex;

    if (!sharedMemory.attach()) {
        qDebug() << "Unable to attach to shared memory segment.";
        return 1;
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    cv::Mat img;

    forever {
        mutex.lock();
        sharedMemory.lock();
        QByteArray byteArray((const char*)sharedMemory.constData(), sharedMemory.size());
        sharedMemory.unlock();
        mutex.unlock();

        buffer.setData(byteArray);
        buffer.open(QBuffer::ReadOnly);
        in >> img; // Deserialize

        if (!img.empty()) {
            imshow("image from shared memory", img);
            cv::waitKey(1);
        }
    }

    return a.exec();
}
