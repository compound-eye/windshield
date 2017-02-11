#include "CameraCalibrations.h"


static double intrinsicM[3*3] = {
    229.823474215748, 0, 157.7769882237044,
    0, 230.4906171005828, 83.10050380838153,
    0, 0, 1,
};
const cv::Mat_<double> intrinsic(3, 3, intrinsicM);

static double distortionM[5] = {
    -0.3930863374131224, 0.2466529498964713, 0, 0, -0.100205391233397,
};
const cv::Mat_<double> distortion(1, 5, distortionM);

static double perspectiveM[3*3] = {
     0.4797382619221335,   - 1.115780385039547,   88.52141341819897,
    -0.008023667698703328,   0.978215301978579,  -41.15293886293881,
     0.0001308960183098493, -0.006837867987023164, 1,
};
const cv::Mat_<double> perspective(3, 3, perspectiveM);
