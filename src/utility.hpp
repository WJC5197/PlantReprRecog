#pragma once
#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include "stdc++.h"
#include "opencv.h"

namespace fs = std::filesystem;

////// multi-thread
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
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime).count();
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();
    T& workload;
};

///// filesystem
std::string relative2AbsPath(std::string relativePath)
{
    return (fs::current_path() / fs::path(relativePath)).generic_string();
}
///// img process
cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 20), cv::Point(-1, -1));

std::string opencvDataType2Str(int type) {
  std::string r;

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

// otsu
int otsu(cv::Mat& img)
{
    ////// 超绿灰度分割
    cv::Mat channel[3];
    cv::split(img, channel);
    channel[1] = 2 * channel[1] - channel[0] - channel[2];
    int otsuThres = cv::threshold(channel[1], img, 0, 255, cv::THRESH_OTSU);
    cv::morphologyEx(img, img, cv::MORPH_CLOSE, dilateKernel);
    //dilate(img, img, dilateKernel);
    return otsuThres;
}

// need to stuck when the program runs to end
void imgWin(cv::Mat img, std::string name, int w = 640, int h = 480) 
{
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::imshow(name, img);
}

// get the percentage of lightness from a gray image
// use opencv builtin api
double getLightness(cv::Mat& img) 
{
    return cv::mean(img)[0] / 255.0;
}

//求Mat的中位数
int matMedian(cv::Mat& img)
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
void cannyMinmaxThres(cv::Mat& img, int& minval, int& maxval, float sigma)
{
  int midval = matMedian(img);
  std::cout << "midval:" << midval << std::endl;
  // 计算低阈值
  minval = cv::saturate_cast<uchar>((1.0 - sigma) * midval);
  //计算高阈值
  maxval = cv::saturate_cast<uchar>((1.0 + sigma) * midval);
}

#endif
