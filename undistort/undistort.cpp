#include <opencv2/opencv.hpp>
#include <iostream>


int main(int argc, char* argv[]) {
    cv::Size board(9, 6), imageSize(320, 240);

    std::vector<cv::Point3f> boardPoints;
    boardPoints.reserve(6*9);
    for (int j = 1; j <= 6; ++j) {
        float y = 32. * j + 8.;
        for (int i = 1; i <= 9; ++i) {
            float x = 32. * i;
            boardPoints.push_back(cv::Point3f(x, y, 0.));
        }
    }

    std::vector<std::vector<cv::Point3f> > objectPoints;
    std::vector<std::vector<cv::Point2f> > imagePoints;

    cv::VideoCapture cap(std::string(getenv("HOME")) + "/Pictures/chessboard0/%04d.jpg", cv::CAP_IMAGES);
    cv::Mat image;
    while (cap.read(image)) {
        cv::resize(image, image, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
        std::vector<cv::Point2f> corners;
        if (cv::findChessboardCorners(image, board, corners)) {
            cv::Mat(corners) *= float(imageSize.width) / image.cols;
            imagePoints.push_back(corners);
            objectPoints.push_back(boardPoints);
        }
    }
    std::cerr << "Collected " << imagePoints.size() << " board samples." << std::endl;

    cv::Mat intrinsic, distortion;
    double err = cv::calibrateCamera(objectPoints, imagePoints, imageSize, intrinsic, distortion,
                                     cv::noArray(), cv::noArray(), cv::CALIB_ZERO_TANGENT_DIST);
    std::cerr << "Reprojection error: " << err << std::endl;
    std::cerr << "Intrinsic: " << intrinsic << std::endl;
    std::cerr << "Distortion: " << distortion << std::endl;

    return 0;
}
