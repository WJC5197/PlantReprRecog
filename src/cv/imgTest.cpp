// rtt means "real time test"
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/ximgproc/segmentation.hpp"
#include "../util/stdc++.h"
#include "../util/utility.hpp"
#include "phm.hpp"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    Mat img = imread("../img/1.jpg");
    Mat painted = img.clone();
    // split img into 3 channels
    vector<cv::Mat> channels;
    cv::split(img, channels);
    thresSeg(img, 1);
    // merge 3 channels, but green is img
    channels[0] = 0; 
    channels[1] = img;
    channels[2] = 0;
    cv::merge(channels, img);
    auto rects = getPlantRegion(img);
    filtrateRegion(rects);
    for (auto rect : rects)
    {
        rectangle(img, rect, Scalar(0, 0, 255), 2);
    }
    imgWin(img, "img");
    waitKey(0);
    return 0;
}