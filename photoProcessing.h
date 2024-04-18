#ifndef PHOTOPROCESSING_H
#define PHOTOPROCESSING_H


#include <opencv2/opencv.hpp>

class PhotoProcessor {
public:
    // Function to crop the image to a specified box
    static cv::Mat cropToBox(const cv::Mat& image, int x, int y, int width, int height);
};

#endif // PHOTOPROCESSING_H
