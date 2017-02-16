#include "RoverParms.h"
#include <opencv2/core/persistence.hpp>


static void ReadMatrix(double* m, int count, const cv::FileNode& node) {
    for (int i = 0; i < count; ++i) {
        m[i] = node[i];
    }
}

void RoverParms::Read(const std::string& filepath) {
    cv::FileStorage file(filepath, cv::FileStorage::READ);
    if (file.isOpened()) {
        double perspective_[3*3];
        ReadMatrix(perspective_, 3*3, file["perspective"]);
        perspective = cv::Mat_<double>(3, 3, perspective_);
    }
}

void RoverParms::Write(const std::string& filepath) {
    cv::FileStorage file(filepath, cv::FileStorage::WRITE|cv::FileStorage::FORMAT_YAML);
    file.write("perspective", perspective);
}
