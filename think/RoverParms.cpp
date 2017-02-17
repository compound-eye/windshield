#include "RoverParms.h"
#include <opencv2/core/persistence.hpp>


void RoverParms::Read(const std::string& filepath) {
    cv::FileStorage file(filepath, cv::FileStorage::READ);
    if (file.isOpened()) {
        perspective = file["perspective"].mat();
    }
}

void RoverParms::Write(const std::string& filepath) {
    cv::FileStorage file(filepath, cv::FileStorage::WRITE|cv::FileStorage::FORMAT_YAML);
    file.write("perspective", perspective);
}
