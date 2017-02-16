#include "ImageLogger.h"
#include <opencv2/imgcodecs.hpp>
#include <sys/stat.h>
#include <time.h>


ImageLogger::ImageLogger(const std::string& parentPath) {
    char dname[100];
    time_t now = time(NULL);
    strftime(dname, sizeof dname, "/%F-%H%M%S", localtime(&now));

    mkdir(parentPath.c_str(), ACCESSPERMS);
    directory = parentPath + dname;
    mkdir(directory.c_str(), ACCESSPERMS);
}

void ImageLogger::BackgroundLoop() {
    for (int i = 0; ! imagesToLog.quitting; ++i) {
        cv::Mat image = imagesToLog.Dequeue();
        if (! image.empty()) {
            char fname[100];
            sprintf(fname, "%s/%04d.png", directory.c_str(), i);
            cv::imwrite(fname, image);
        }
    }
}

static void* ImageLoggerThread(void* logger) {
    ((ImageLogger*)logger)->BackgroundLoop();
    return 0;
}

void ImageLogger::Start() {
    pthread_create(&thread, NULL, ImageLoggerThread, this);
}

void ImageLogger::Stop() {
    imagesToLog.Quit();
    pthread_join(thread, NULL);
}
