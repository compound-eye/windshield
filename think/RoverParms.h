#ifndef ROVERPARMS_H
#define ROVERPARMS_H

#include <opencv2/core.hpp>


class RoverParms {
public:
    cv::Mat perspective;

    void Read(const std::string& filepath);
    void Write(const std::string& filepath);
};

#endif // ROVERPARMS_H
