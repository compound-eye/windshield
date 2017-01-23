#include <opencv2/opencv.hpp>
#include <iostream>


int main(int argc, char* argv[]) {
    char srcfile[200];
    strcpy(srcfile, __FILE__);
    std::string dir(srcfile, strrchr(srcfile,'/')+1);

    cv::Mat image = cv::imread(dir + "pic3.jpg");

    cv::Size board(9, 6), dst(320, 240);
    double scale = double(image.size().width) / dst.width;

    std::vector<cv::Point2f> corners;
    bool found = cv::findChessboardCorners(image, board, corners);

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::cornerSubPix(gray, corners, cv::Size(11,11), cv::Size(-1,-1),
                     cv::TermCriteria(cv::TermCriteria::EPS|cv::TermCriteria::COUNT, 30, 0.1));

    cv::Point2f obj[4], img[4];
    img[0] = corners[0];
    img[1] = corners[board.width - 1];
    img[2] = corners[board.height*board.width - board.width];
    img[3] = corners[board.height*board.width - 1];
    for (int i = 0; i < 4; ++i) {
        img[i].x /= scale;
        img[i].y /= scale;
    }
    obj[0] = img[0];
    obj[1].x = img[1].x; obj[1].y = img[0].y;
    obj[2].x = img[0].x; obj[2].y = img[0].y + (img[1].x - img[0].x) * (board.height-1) / (board.width-1);
    obj[3].x = img[1].x; obj[3].y = obj[2].y;

    cv::circle(image, img[0], 9, cv::Scalar(255, 0, 0), 3);
    cv::circle(image, img[1], 9, cv::Scalar(0, 255, 0), 3);
    cv::circle(image, img[2], 9, cv::Scalar(0, 0, 255), 3);
    cv::circle(image, img[3], 9, cv::Scalar(0,255,255), 3);
    cv::drawChessboardCorners(image, board, corners, found);
    cv::imwrite(dir + "pic-mark.jpg", image);

    cv::Mat H = cv::getPerspectiveTransform(img, obj);
    std::cout << H << std::endl;

    cv::resize(image, image, dst);
    cv::warpPerspective(image, image, H, image.size());
    cv::imwrite(dir + "pic-top.jpg", image);

    return 0;
}
