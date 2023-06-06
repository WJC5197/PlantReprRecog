#pragma once
#ifndef _PLANT_MEASURE_HEIGHT_
#define _PLANT_MEASURE_HEIGHT_

#include "../util/opencv.h"
#include "../util/stdc++.h"
#include "../util/utility.hpp"

using namespace cv;
using namespace std;
using namespace cv::ximgproc::segmentation;

namespace fs = std::filesystem;

//// Calibrate

//// img process
cv::Mat dilateKernel = getStructuringElement(MORPH_RECT, Size(10, 20), Point(-1, -1));
cv::Mat closeKernel = getStructuringElement(MORPH_RECT, Size(10, 20), Point(-1, -1));
int thresSeg(Mat &, bool);
double lightRegionMeanMaxHeight(Mat &);

//// Region Proposal
int resizeHeight = 200;
vector<Rect> getPlantRegion(Mat &);
double iou(const Rect &, const Rect &);
void filtrateRegion(vector<Rect> &);

//// kmeans
const int K = 1;         // number of clusters
const int MAX_ITER = 10; // maximum number of iterations
vector<Point2f> centers;
double distance(const Point2f &, const Point2f &);
vector<Point2f> kmeans(const vector<Point2f> &, int, int);
void findCenters(Mat &, vector<Point2f> &);
// auto kmeansWork = Work<Mat>(img);

//// findContours
vector<vector<Point>> contours;
vector<Vec4i> hierarchy;
vector<vector<Point>> filtratedContours;
double areaThres = 60000;
void findPlantContours(Mat &, vector<Vec4i> &, vector<vector<Point>> &);
// auto contoursWork = Work<Mat>(img);

//// heightMeasure
tuple<int, int> getPlantHeight(const Point2f &, const Mat &, int, int);

//// stitch
bool tryUseGPU = false;
bool divideImg = false;
Stitcher::Mode mode = Stitcher::PANORAMA;
vector<Mat> imgs;
string resultName = "result.jpg";
void printUsage(char **argv);
int parseStitchArgs(int argc, char **argv);
int stitch(int, char *[]);

int thresSeg(Mat &img, bool rawRepre = 0)
{
    // ////// White balance, use opencv api
    // cv::Ptr<cv::xphoto::LearningBasedWB> wb = cv::xphoto::createLearningBasedWB();
    // // set Saturation
    // wb->setSaturationThreshold(0.5);
    // wb->balanceWhite(img, img);
    // cv::Ptr<cv::xphoto::GrayworldWB> wb = cv::xphoto::createGrayworldWB();

    // 对图像进行白平衡校正
    // wb->balanceWhite(img, img);

    ////// 超绿灰度分割
    // convert img to 3 channel int
    img.convertTo(img, CV_32S);
    Mat channel[3];
    split(img, channel);
    channel[1] = 2 * channel[1] - channel[0] - channel[2];
#pragma omp parallel for
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            if (channel[1].at<int>(i, j) < 30)
            {
                channel[1].at<int>(i, j) = 0;
            }
            else if (channel[1].at<int>(i, j) > 255)
            {
                channel[1].at<int>(i, j) = 255;
            }
        }
    }
    channel[1].convertTo(channel[1], CV_8U);
    morphologyEx(channel[1], channel[1], MORPH_CLOSE, closeKernel);
    if (rawRepre)
    {
        img = channel[1];
        return 0;
    }
    else
    {
        int otsuThres = threshold(channel[1], img, 0, 255, cv::THRESH_OTSU);
        // dilate(img, img, dilateKernel);
        return otsuThres;
    }
}

double lightRegionMeanMaxHeight(Mat &img)
{
    double meanHeight = 0;
    int cnt = 0;
// iterate every column of img to find the max height, do this to all img, and return every column's max height mean    double meanHeight = 0;
#pragma omp parallel for
    for (int i = 0; i < img.cols; i++)
    {
        int maxH = 0;
        for (int j = 0; j < img.rows; j++)
        {
            if (img.at<uchar>(j, i) == 255)
            {
                maxH = j;
                cnt++;
                break;
            }
        }
        meanHeight += maxH;
    }
    if (cnt == 0)
        return 0;
    else
        return img.cols - (meanHeight / cnt);
}

// distance measure
double distance(const Point2f &p, const Point2f &center)
{
    return sqrt(3 * pow(p.x - center.x, 2) + pow(p.y - center.y, 2));
}

// k-means algorithm
vector<Point2f> kmeans(const vector<Point2f> &points, int k, int maxIter)
{
    // initialize cluster centers randomly
    vector<Point2f> centers;
    for (int i = 0; i < k; i++)
    {
        int idx = rand() % points.size();
        centers.push_back(points[idx]);
    }

// run k-means algorithm
#pragma omp parallel for
    for (int iter = 0; iter < maxIter; iter++)
    {
        // assign each data point to the closest cluster center
        vector<vector<Point2f>> clusters(k);
        for (const auto &p : points)
        {
            double min_dist = numeric_limits<double>::max();
            int minIdx = 0;
            for (int i = 0; i < k; i++)
            {
                double dist = distance(p, centers[i]);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    minIdx = i;
                }
            }
            clusters[minIdx].push_back(p);
        }

// update each cluster center as the mean of its data points
#pragma omp parallel for
        for (int i = 0; i < k; i++)
        {
            Point2f mean(0, 0);
            for (const auto &p : clusters[i])
            {
                mean += p;
            }
            if (!clusters[i].empty())
            {
                mean /= static_cast<float>(clusters[i].size());
            }
            centers[i] = mean;
        }
    }

    return centers;
}

void findCenters(Mat &workload, vector<Point2f> &centers)
{
    // collect all grey pixels as data points
    vector<Point2f> points;
    for (int i = 0; i < workload.rows; i++)
    {
        for (int j = 0; j < workload.cols; j++)
        {
            if (workload.at<uchar>(i, j) > 0)
            {
                points.push_back(Point2f(j, i));
            }
        }
    }
    centers = kmeans(points, K, MAX_ITER);
    // cout << "||>>thread:" << centers.size() << endl;
}

void findPlantContours(Mat &workload, vector<Vec4i> &hierarchy, vector<vector<Point>> &filtratedContours)
{
    ////// findcontours
    /*
        void findContours(
            cv::InputOutputArray image, // 输入的8位单通道“二值”图像
            cv::OutputArrayOfArrays contours, // 包含points的vectors的vector
            cv::OutputArray hierarchy, // (可选) 拓扑信息
                RETR_LIST:没有父子结构,只是单纯的边界结构,属于同一层
                RETR_EXTWENAL:只返回外层边界
                RETR_CCOMP:返回全部的边界
            int mode, // 轮廓检索模式
            int method, // 近似方法
            cv::Point offset = cv::Point() // (可选) 所有点的偏移
        );
    */

    findContours(
        workload,
        contours,
        hierarchy,
        RETR_TREE,
        CHAIN_APPROX_SIMPLE);
    cout << "|>contours' size:" << contours.size() << endl;
    ////// filtering contours by threshold area
    double area;
    for (int i = 0; i < contours.size(); i++)
    {
        area = contourArea(contours[i]);
        if (area > areaThres)
        {
            filtratedContours.push_back(contours[i]);
        }
    }
    cout << "|>filtrated contours' size:" << filtratedContours.size() << endl;
}

tuple<int, int> getPlantHeight(const Point2f &center, const Mat &img, int checkDis, int lowestTime)
{
    int highest = 0;
    int lowest = 0;
    int x = center.x;
    int y = center.y;
    int width = img.cols;
    int height = img.rows;
    // get highest from
    for (int i = 0; i < y; i++)
    {
        if (img.at<bool>(i, x) > 0)
        {
            highest = i;
            break;
        }
    }
    // get lowest, considering sloping plant
    int dir;
    // left & right dis px check if the height between center & column lowest is increasing
    int leftSum = 0, rightSum = 0;
    for (int i = 0; i < checkDis; i++)
    {
        int j = 1, k = 1;
        while (img.at<bool>(height - j, x - i) == 0)
        {
            j++;
            if (height - j < 0)
                break;
        }
        leftSum += j;
        while (img.at<bool>(height - k, x + i) == 0)
        {
            k++;
            if (height - k < 0)
                break;
        }
        rightSum += k;
    }
    dir = leftSum < rightSum ? -1 : 1; // true is left, false is right
    // cout << "||>>" << dir << endl;
    int cnt = 0; // if lowest position keeps for lowDis horizition pixel, break out of loop
    int i = 0, j = 1;
    while (x + i * dir < width && x + i * dir >= 0)
    {
        while (height - j > y && img.at<bool>(height - j, x + i * dir) == 0)
        {
            j++;
        }
        if (cnt == lowestTime)
        {
            break;
        }
        if (height - j > lowest)
        {
            lowest = height - j;
        }
        else
        {
            cnt++;
        }
        i++;
        j = 1;
        // cout << "||>>" << x + i * dir << endl;
    }
    return tuple<int, int>(highest, lowest);
}

vector<Rect> getPlantRegion(Mat &img)
{
    // speed-up using multithreads
    setUseOptimized(true);
    setNumThreads(8);

    // resize image
    int resizeWidth = img.cols * resizeHeight / img.rows;
    resize(img, img, Size(resizeWidth, resizeHeight));

    // create Selective Search Segmentation Object using default parameters
    Ptr<SelectiveSearchSegmentation> ss = createSelectiveSearchSegmentation();
    // set input image on which we will run segmentation
    ss->setBaseImage(img);

    // Switch to high recall but slow Selective Search method
    ss->switchToSelectiveSearchQuality();

    // run selective search segmentation on input image
    std::vector<Rect> rects;
    ss->process(rects);
    std::cout << "Total Number of Region Proposals: " << rects.size() << std::endl;

    return rects;
}

double iou(const Rect &r1, const Rect &r2)
{
    int x1 = max(r1.x, r2.x);
    int y1 = max(r1.y, r2.y);
    int x2 = min(r1.x + r1.width, r2.x + r2.width);
    int y2 = min(r1.y + r1.height, r2.y + r2.height);
    int w = max(0, x2 - x1);
    int h = max(0, y2 - y1);
    double inter = w * h;
    double area1 = r1.width * r1.height;
    double area2 = r2.width * r2.height;
    double iou = inter / (area1 + area2 - inter);
    return iou;
}

void filtrateRegion(vector<Rect> &regions)
{
    // filtrate based on iou
    vector<Rect> tmp;
    vector<int> idx(regions.size());
    for (int i = 0; i < idx.size(); i++)
    {
        idx[i] = i;
    }
    for (int i = 0; i < idx.size(); i++)
    {
        int cnt = 0;
        for (int j = i + 1; j < idx.size(); j++)
        {
            if (idx[j] == -1)
                continue;
            if (iou(regions[i], regions[j]) > 0.5)
            {
                cnt++;
                idx[j] = -1;
            }
        }
        if (cnt > 3)
        {
            tmp.push_back(regions[i]);
        }
    }
    regions = tmp;
    std::cout << "Total Number of Region Proposals: " << regions.size() << std::endl;
    // filtrate based on length
    for (int i = 0; i < regions.size(); i++)
    {
        for (int j = 0; j < regions.size(); j++)
        {
            if (i == j)
                continue;
            if (regions[j].x + regions[j].width < regions[i].x + regions[i].width && regions[j].y + regions[j].height < regions[i].y + regions[i].height)
            {
                if (regions[j].x > regions[i].x && regions[j].y > regions[i].y)
                {
                    regions.erase(regions.begin() + j);
                }
            }
        }
    }
    std::cout << "Total Number of Region Proposals: " << regions.size() << std::endl;
}

int stitch(int argc, char *argv[])
{
    int retval = parseStitchArgs(argc, argv);
    if (retval)
        return EXIT_FAILURE;
    Mat pano;
    Ptr<Stitcher> stitcher = Stitcher::create(mode);
    Stitcher::Status status = stitcher->stitch(imgs, pano);
    if (status != Stitcher::OK)
    {
        cout << "Can't stitch images, error code = " << int(status) << endl;
        return EXIT_FAILURE;
    }
    imwrite(resultName, pano);
    cout << "stitching completed successfully\n"
         << resultName << " saved!";
    return EXIT_SUCCESS;
}

void printUsage(char **argv)
{
    cout << "Images stitcher.\n\n"
         << "Usage :\n"
         << argv[0] << " [Flags] img1 img2 [...imgN]\n\n"
                       "Flags:\n"
                       "  --d3\n"
                       "      internally creates three chunks of each image to increase stitching success\n"
                       "  --try_use_gpu (yes|no)\n"
                       "      Try to use GPU. The default value is 'no'. All default values\n"
                       "      are for CPU mode.\n"
                       "  --mode (panorama|scans)\n"
                       "      Determines configuration of stitcher. The default is 'panorama',\n"
                       "      mode suitable for creating photo panoramas. Option 'scans' is suitable\n"
                       "      for stitching materials under affine transformation, such as scans.\n"
                       "  --output <result_img>\n"
                       "      The default is 'result.jpg'.\n\n"
                       "Example usage :\n"
         << argv[0] << " --d3 --try_use_gpu yes --mode scans img1.jpg img2.jpg\n";
}

int parseStitchArgs(int argc, char **argv)
{
    if (argc == 1)
    {
        printUsage(argv);
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; ++i)
    {
        if (string(argv[i]) == "--help" || string(argv[i]) == "/?")
        {
            printUsage(argv);
            return EXIT_FAILURE;
        }
        else if (string(argv[i]) == "--try_use_gpu")
        {
            if (string(argv[i + 1]) == "no")
                tryUseGPU = false;
            else if (string(argv[i + 1]) == "yes")
                tryUseGPU = true;
            else
            {
                cout << "Bad --try_use_gpu flag value\n";
                return EXIT_FAILURE;
            }
            i++;
        }
        else if (string(argv[i]) == "--d3")
        {
            divideImg = true;
        }
        else if (string(argv[i]) == "--output")
        {
            resultName = argv[i + 1];
            i++;
        }
        else if (string(argv[i]) == "--mode")
        {
            if (string(argv[i + 1]) == "panorama")
                mode = Stitcher::PANORAMA;
            else if (string(argv[i + 1]) == "scans")
                mode = Stitcher::SCANS;
            else
            {
                cout << "Bad --mode flag value\n";
                return EXIT_FAILURE;
            }
            i++;
        }
        else
        {
            Mat img = imread(samples::findFile(argv[i]));
            if (img.empty())
            {
                cout << "Can't read image '" << argv[i] << "'\n";
                return EXIT_FAILURE;
            }
            if (divideImg)
            {
                Rect rect(0, 0, img.cols / 2, img.rows);
                imgs.push_back(img(rect).clone());
                rect.x = img.cols / 3;
                imgs.push_back(img(rect).clone());
                rect.x = img.cols / 2;
                imgs.push_back(img(rect).clone());
            }
            else
                imgs.push_back(img);
        }
    }
    return EXIT_SUCCESS;
}
#endif
