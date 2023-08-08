// rtt means "real time test"
#include "../src/cv/phm.hpp"
#include "../src/util/stdc++.h"
#include "../src/util/utility.hpp"
#include "opencv2/ximgproc/segmentation.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    Mat img = imread("../img/1.jpg");
    Mat painted = img.clone();
    // split img into 3 channels
    auto channels = split(img);
    thresSeg(img);
    // merge 3 channels, but green is img
    img = merge(channels[0], img, channels[2]);
    auto rects = getPlantRegion(img);
    filtrateRegion(rects);
    for (auto rect : rects)
    {
        rectangle(painted, rect, Scalar(0, 0, 255), 2);
    }
    imgWin("img", painted);
    waitKey(0);
    return 0;
}