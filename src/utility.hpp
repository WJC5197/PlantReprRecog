#pragma once
#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include "stdc++.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
template <typename T>
class Work
{
public:
    Work() {}
    Work(T& workload_) :workload(workload_) {}
    template<typename... Args>
    void operator()(void(*func)(T&, Args& ...), Args& ... referArgs)
    {
        func(workload, referArgs...);
    }
    void setWorkload(T& workload)
    {
        this->workload = workload;
    }
    double microsTimeCost()
    {
        return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime).count();
    }
private:
    chrono::time_point<chrono::steady_clock> startTime = chrono::steady_clock::now();
    T& workload;
};

string opencvDataType2Str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}
// need to stuck when the program runs to end
void imgWin(Mat img, string name, int w = 640, int h = 480) {
    namedWindow(name, WINDOW_NORMAL);
    imshow(name, img);
}
//求Mat的中位数
int matMedian(Mat& img)
{
    //判断如果不是单通道直接返回128
    if (img.channels() > 1) return 128;
    int rows = img.rows;
    int cols = img.cols;
    //定义数组
    float mathists[256] = { 0 };
    //遍历计算0-255的个数
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int val = img.at<uchar>(row, col);
            mathists[val]++;
        }
    }

    int calcval = rows * cols / 2;
    int tmpsum = 0;
    for (int i = 0; i < 255; ++i) {
        tmpsum += mathists[i];
        if (tmpsum > calcval) {
            return i;
        }
    }
    return 0;
}
//求自适应阈值的最小和最大值
void cannyMinmaxThres(Mat& img, int& minval, int& maxval, float sigma)
{
  int midval = matMedian(img);
  cout << "midval:" << midval << endl;
  // 计算低阈值
  minval = saturate_cast<uchar>((1.0 - sigma) * midval);
  //计算高阈值
  maxval = saturate_cast<uchar>((1.0 + sigma) * midval);
}
// hsv
// (H-0~180;S-0~255;V-0~255)
#endif