#ifndef _PLANT_MEASURE_HEIGHT_
#define _PLANT_MEASURE_HEIGHT_

#include "stdc++.h"
#include "opencv.h"
#include "utility.hpp"

// ImgSize
#define HEIGHT (img.rows)
#define WIDTH (img.cols)

using namespace cv;
using namespace std;

namespace fs = std::filesystem;

//// ImgTest
Mat img = imread((fs::current_path() / fs::path("../img/home1.jpg")).generic_string());
//Mat img = imread("D:\\_Proj\\CC\\PlantReprRecog\\img\\home1.jpg");
Mat raw = img; // raw is the clone of img
//// Calibrate

//// img preprocess

//// kmeans
const int K = 3;  // number of clusters
const int MAX_ITER = 10;  // maximum number of iterations
vector<Point2f> centers;
double distance(const Point2f&, const Point2f&);
vector<Point2f> kmeans(const vector<Point2f>&, int, int);
void findCenters(Mat&, vector<Point2f>&);
auto kmeansWork = Work<Mat>(img);

//// findContours
vector<vector<Point>> contours;
vector<Vec4i> hierarchy;
vector<vector<Point>> filtratedContours;
double areaThres = 60000;
void findPlantContours(Mat&, vector<Vec4i>&, vector<vector<Point>>&);
auto contoursWork = Work<Mat>(img);

//// heightMeasure
tuple<int, int> getPlantHeight(const Point2f&, const Mat&, int, int);

//// stitch
bool tryUseGPU = false;
bool divideImg = false;
Stitcher::Mode mode = Stitcher::PANORAMA;
vector<Mat> imgs;
string resultName = "result.jpg";
void printUsage(char** argv);
int parseStitchArgs(int argc, char** argv);
int stitch(int, char* []);

/* Camera Main */
//int main() {
//    // initialize
//    VideoCapture camera;
//    camera.open(1);    // 打开摄像头, 默认摄像头cameraIndex=0
//    if (!camera.isOpened())
//    {
//        cerr << "Couldn't open camera." << endl;
//    }
//    // camera args
//    camera.set(CAP_PROP_FRAME_WIDTH, 640);      // 宽度
//    camera.set(CAP_PROP_FRAME_HEIGHT, 480);    // 高度
//    camera.set(CAP_PROP_FPS, 10);               // 帧率
//
//    // frame args
//    double frame_width = camera.get(CAP_PROP_FRAME_WIDTH);
//    double frame_height = camera.get(CAP_PROP_FRAME_HEIGHT);
//    double fps = camera.get(CAP_PROP_FPS);
//
//    Mat frame;
//    while (true)
//    {
//        camera >> frame;
//        imshow("video", frame);
//        if (waitKey(33) == 27) break;   // ESC 键退出
//    }
//    // 释放
//    camera.release();
//    return 0;
//}
// draw main
//int main()
//{
//    // read input image
//    cout << "||>> image dim:" << HEIGHT << "," << WIDTH << endl;
//    if (img.empty())
//    {
//        cerr << "Unable to read input image" << endl;
//        return 1;
//    }

//	int otsuThres = otsu(img);
//    imgWin(img, "Preprocessed Img");
//    thread t0([&]() {kmeansWork(findCenters, centers); });
//    thread t1([&]() {contoursWork(findPlantContours, hierarchy, filtratedContours); });
//    t0.join();
//    t1.join();
//    cout << "||>> kmeans time cost:" << kmeansWork.microsTimeCost() << endl;
//    cout << "||>> findContours time cost:" << contoursWork.microsTimeCost() << endl;

//    //cout << "||>>main" << centers.size() << endl;
//    // draw centers on output image
//    for (const auto& center : centers)
//    {
//        cout << "||>> center:" << center.x << "," << center.y << endl;
//        circle(raw, Point(center.x, center.y), 15, Scalar(255,0,0), -1);
//        auto tup = getPlantHeight(center, img, 50, 400);
//        line(raw, Point(center.x, get<0>(tup)), Point(center.x, get<1>(tup)), Scalar(0, 255, 0), 3, LINE_8);
//    }
//    // display output image
//    imgWin(raw, "res");
//    waitKey(0);
//    return 0;
//}
double lightRegionMeanMaxHeight(Mat &img)
{
    double meanHeight = 0;
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
                break;
            }
        }
        meanHeight += maxH;
    }
    return HEIGHT - (meanHeight / img.cols);
}

double mapCycleToHeight(double cycle)
{
    return cycle / 400 * 0.8;
}

// distance measure
double distance(const Point2f& p, const Point2f& center)
{
    return sqrt(3 * pow(p.x - center.x, 2) + pow(p.y - center.y, 2));
}

// k-means algorithm
vector<Point2f> kmeans(const vector<Point2f>& points, int k, int maxIter)
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
        for (const auto& p : points)
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
            for (const auto& p : clusters[i])
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

void findCenters(Mat& workload, vector<Point2f>& centers) 
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
    //cout << "||>>thread:" << centers.size() << endl;
}

void findPlantContours(Mat& workload, vector<Vec4i>& hierarchy, vector<vector<Point>>& filtratedContours)
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
        CHAIN_APPROX_SIMPLE
    );
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

// checkDis:
// lowestTime:
tuple<int, int> getPlantHeight(const Point2f& center, const Mat& img, int checkDis, int lowestTime)
{
    int highest = 0;
    int lowest = 0;
    int x = center.x;
    int y = center.y;
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
        while (img.at<bool>(HEIGHT - j, x - i) == 0)
        {
            j++;
        }
        leftSum += j;
        while (img.at<bool>(HEIGHT - k, x + i) == 0)
        {
            k++;
        }
        rightSum += k;
    }
    dir = leftSum < rightSum ? -1 : 1; // true is left, false is right
    //cout << "||>>" << dir << endl;
    int cnt = 0; // if lowest position keeps for lowDis horizition pixel, break out of loop
    int i = 0, j = 1;
    while (1) {
        while (HEIGHT - j > y && img.at<bool>(HEIGHT - j, x + i * dir) == 0)
        {
            j++;
        }
        if (cnt == lowestTime)
        {
            break;
        }
        if (HEIGHT - j > lowest) {
            lowest = HEIGHT - j;
        }
        else {
            cnt++;
        }
        i++;
        j = 1;
        //cout << "||>>" << x + i * dir << endl;
    }
    return tuple<int, int>(highest, lowest);
}

int stitch(int argc, char* argv[])
{
    int retval = parseStitchArgs(argc, argv);
    if (retval) return EXIT_FAILURE;
    Mat pano;
    Ptr<Stitcher> stitcher = Stitcher::create(mode);
    Stitcher::Status status = stitcher->stitch(imgs, pano);
    if (status != Stitcher::OK)
    {
        cout << "Can't stitch images, error code = " << int(status) << endl;
        return EXIT_FAILURE;
    }
    imwrite(resultName, pano);
    cout << "stitching completed successfully\n" << resultName << " saved!";
    return EXIT_SUCCESS;
}

void printUsage(char** argv)
{
    cout <<
        "Images stitcher.\n\n" << "Usage :\n" << argv[0] << " [Flags] img1 img2 [...imgN]\n\n"
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
        "Example usage :\n" << argv[0] << " --d3 --try_use_gpu yes --mode scans img1.jpg img2.jpg\n";
}

int parseStitchArgs(int argc, char** argv)
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
