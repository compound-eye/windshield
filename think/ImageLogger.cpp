#include "ImageLogger.h"
#include <opencv2/imgcodecs.hpp>
#include <sys/stat.h>
#include <time.h>


void ImageLogger::BackgroundLoop() {
    char fname[100];

    time_t now = time(NULL);
    strftime(fname, sizeof fname, "/%F-%H%M%S", localtime(&now));

    static const char* pathLogHomeDir = "/home/pi/rover-images";
    mkdir(pathLogHomeDir, ACCESSPERMS);
    std::string pathLogDir = std::string(pathLogHomeDir) + fname;
    mkdir(pathLogDir.c_str(), ACCESSPERMS);

    for (int i = 0; ! imagesToLog.quitting; ++i) {
        cv::Mat image = imagesToLog.Dequeue();
        if (! image.empty()) {
            sprintf(fname, "%s/%03d.png", pathLogDir.c_str(), i);
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
