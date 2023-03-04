//#include <opencv2/opencv.hpp>
//#include <opencv2/opencv_modules.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <iostream>
//#include <chrono>
//#include <thread>
//#include <tuple>
//
//#include "utility.hpp"
//
//using namespace cv;
//using namespace std;
////global variable
//string window_name = "camera"; // showed window's name
//int screen_ratio; // the ratio
//int screen_width_max = 600;
//chrono::microseconds thread1_duration;
//chrono::microseconds thread2_duration;
//
//// thresholds
//int green_h_low_th = 45;
//int green_s_low_th = 45;
//int green_v_low_th = 0;
//int green_h_high_th = 75;
//int green_s_high_th = 150;
//int green_v_high_th = 255;
//
//// red is separate,need special treatment
//int upper_red_h_low_th = 0;
//int lower_red_h_low_th = 170;
//int red_s_low_th = 15;
//int red_v_low_th = 0;
//
//int upper_red_h_high_th = 10;
//int lower_red_h_high_th = 179;
//int red_s_high_th = 255;
//int red_v_high_th = 255;
//
//Mat convolution_kernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
//vector<Vec4i> hierarchy; // findContours
//vector<vector<Point>> contours;
//
//class red_ball_extract
//{
//public:
//	chrono::time_point<chrono::steady_clock> start_time;
//	chrono::time_point<chrono::steady_clock> end_time;
//	Mat& hsv; // must be hsv image
//	red_ball_extract(Mat& hsv_):hsv(hsv_){}
//	void operator()()
//	{
//		start_time = chrono::steady_clock::now();
//		Mat upper_red = hsv;
//		Mat lower_red = hsv;
//		inRange(upper_red,
//				Scalar(upper_red_h_low_th,red_s_low_th,red_s_low_th),
//				Scalar(upper_red_h_high_th,red_s_high_th,red_v_high_th),
//				upper_red);
//		inRange(lower_red,
//				Scalar(lower_red_h_low_th,red_s_low_th,red_s_low_th),
//				Scalar(lower_red_h_high_th,red_s_high_th,red_v_high_th),
//				lower_red);
//		addWeighted(upper_red, 1.0, lower_red, 1.0, 0.0, hsv);
//		morphologyEx(hsv, hsv, MORPH_OPEN, convolution_kernel);
//        morphologyEx(hsv, hsv, MORPH_CLOSE, convolution_kernel);
//		Canny(hsv, hsv, 20, 80);
//		findContours(hsv,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_NONE);
//		end_time = chrono::steady_clock::now();
//		thread1_duration = chrono::duration_cast<chrono::microseconds>(end_time-start_time);
//	}
//};
//
//class plant_extract
//{
//public:
//	chrono::time_point<chrono::steady_clock> start_time;
//	chrono::time_point<chrono::steady_clock> end_time;
//	Mat& hsv;
//	int upperMostPos;
//	int downMostPos;
//	plant_extract(Mat& hsv_):hsv(hsv_){}
//	tuple<int,int> operator()()
//	{
//		start_time = chrono::steady_clock::now();
//		int cols = hsv.cols;
//		int rows = hsv.rows;
//		inRange(hsv,
//				Scalar(green_h_low_th,green_s_low_th,green_s_low_th),
//				Scalar(green_h_high_th,green_s_high_th,green_v_high_th),
//				hsv);
//		morphologyEx(hsv, hsv, MORPH_OPEN, convolution_kernel);
//        morphologyEx(hsv, hsv, MORPH_CLOSE, convolution_kernel);
//		for(int i=0;i<rows;i++)
//		{
//			for(int j=0;j<cols;j++)
//			{
//				if(hsv.at<uchar>(i,j)==255)
//				{
//					upperMostPos = j;
//					break;
//				}
//			}
//		}
//		for(int i=rows-1;i>0;i--)
//		{
//			for(int j=cols-1;j>0;j--)
//			{
//				if(hsv.at<uchar>(i,j)==255)
//				{
//					downMostPos = j;
//				}
//			}
//		}
//		end_time = chrono::steady_clock::now();
//		thread2_duration = chrono::duration_cast<chrono::microseconds>(end_time-start_time);
//		return tuple<int,int>(upperMostPos,downMostPos);
//	}
//};
//
//void circle_recog(vector<vector<Point>> &contours,vector<vector<Point>> &save_contours,int &i,Mat &frame,bool &anchor,double &roundness)
//{
//	// filter the small contours
//	double epilson = 0.01 * arcLength(contours[i],false);
//	save_contours = contours;
//	approxPolyDP(save_contours[i],contours[i],epilson ,true);
//	//筛除面积很小的轮廓
//	double area = contourArea(contours[i], true);
//	roundness = -1;
//	if (area<750)
//	{
//		contours[i].clear();
//		return;
//	}
//	auto corners = contours[i].size();
//	auto moment = moments(contours[i]);
//	double x = moment.m10/moment.m00;
//	double y = moment.m01/moment.m00;
//	double x_left = contours[i][0].x;
//	double x_right =contours[i][0].x ;
//	double y_up = contours[i][0].y;
//	double y_down = contours[i][0].y;
//	for (int j=0;j<contours[i].size();j++)
//	{
//		if (contours[i][j].x<x_left)
//		{
//			x_left = contours[i][j].x;
//		}
//		else if(contours[i][j].x>x_right)
//		{
//			x_right = contours[i][j].x;
//		}
//		else if (contours[i][j].y<y_up)
//		{
//			y_up = contours[i][j].y;
//		}
//		else if (contours[i][j].y>y_down)
//		{
//			y_down = contours[i][j].y;
//		}
//	}
//	if(corners>=15) // judge whether the contour is a circle
//	{
//		roundness = epilson / area;
//		anchor = 1;
//		circle(frame, Point2d(x, y), 3, Scalar(0, 255, 0), 3);
//		rectangle(frame,Rect2d(x_left,y_up,x_right-x_left,y_down-y_up),Scalar(0,0,255),2);
//		//通过圆度来判断是否为圆形
//		//圆度的计算公式=周长^2/面积
//	}
//}
//int main() {
//	// initialize
//	VideoCapture camera;
//	camera.open(0);    // 打开摄像头, 默认摄像头cameraIndex=0
//	if (!camera.isOpened())
//	{
//		cerr << "Couldn't open camera." << endl;
//	}
//	// camera args
//	camera.set(CAP_PROP_FRAME_WIDTH, 1000);      // 宽度
//	camera.set(CAP_PROP_FRAME_HEIGHT, 1000);    // 高度
//	camera.set(CAP_PROP_FPS, 30);               // 帧率
//
//	// frame args
//	double frame_width = camera.get(CAP_PROP_FRAME_WIDTH);
//	double frame_height = camera.get(CAP_PROP_FRAME_HEIGHT);
//	double fps = camera.get(CAP_PROP_FPS);
//
//	int count_empty = 0;
//	bool anchor=0;
//	double roundness = -1;
//	double area = 0;
//	// 循环读取视频帧
//	vector<vector<Point>> save_contours;
//	Mat frame; // 
//	// Mat gray; // gray img, rgb2gray
//	Mat hsv;
//	vector<Mat> channels;
//	Mat process;
//	while (true)
//	{
//		count_empty = 0;
//		camera >> frame;
//		cvtColor(frame,hsv,COLOR_BGR2HSV); //hsv img
//		frame = hsv; // which pass to one subprocess
//		// cvtColor(frame,gray,COLOR_BGR2GRAY); //gray img
//		split(hsv,channels); // get Channels
//		// GaussianBlur(process,process,Size(3,3),0); 
//		Canny(process, process, 20, 80);
//		//threshold(process,process,125,255,THRESH_BINARY_INV);
//		findContours(process,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_NONE);
//		for (int i=0;i<contours[i].size();i++)
//		{
//			circle_recog(contours,save_contours,i,frame,anchor,roundness);
//			if (contours[i].empty())
//			{
//				count_empty++;
//				continue;
//			}
//			drawContours(frame, contours, i, Scalar(150,0,78),1,8,hierarchy);
//		}
//		putText(frame,"Contours num:"+to_string(contours.size()),Point2d(25,50),FONT_HERSHEY_PLAIN,2,Scalar(0,0,255),2);
//		putText(frame,"Filtered contours:"+to_string(count_empty),Point2d(25,75),FONT_HERSHEY_PLAIN,2,Scalar(0,0,255),2);
//		//putText(frame, "Roundness:" + to_string(roundness), Point2d(25, 100), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2);
//		imshow(window_name, frame);
//		if (anchor)
//		{
//			waitKey(1000);
//			anchor = 0;
//		}
//		if (waitKey(33) == 27) break;   // ESC 键退出
//	}
//	// 释放
//	camera.release();
//	destroyWindow(window_name);
//	return 0;
//}

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "stdc++.h"
#include "utility.hpp"

#define HEIGHT (img.rows)
#define WIDTH (img.cols)

using namespace cv;
using namespace std;


Mat img = imread("D:\\_Proj\\CC\\PlantReprRecog\\img\\home1.jpg");
Mat raw = img; // raw is the clone of img
//// img preprocess
int otsu(Mat&);
Mat dilateKernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
Mat closeKernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));

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

// draw in main
int main()
{
    // read input image
    cout << "||>> image dim:" << HEIGHT << "," << WIDTH << endl;
    if (img.empty())
    {
        cerr << "Unable to read input image" << endl;
        return 1;
    }

	int otsuThres = otsu(img);
    imgWin(img, "Preprocessed Img");
    thread t0([&]() {kmeansWork(findCenters, centers); });
    thread t1([&]() {contoursWork(findPlantContours, hierarchy, filtratedContours); });
    t0.join();
    t1.join();
    cout << "||>> kmeans time cost:" << kmeansWork.microsTimeCost() << endl;
    cout << "||>> findContours time cost:" << contoursWork.microsTimeCost() << endl;

    //cout << "||>>main" << centers.size() << endl;
    // draw centers on output image
    for (const auto& center : centers)
    {
        cout << "||>> center:" << center.x << "," << center.y << endl;
        circle(raw, Point(center.x, center.y), 15, Scalar(255,0,0), -1);
        auto tup = getPlantHeight(center, img, 50, 400);
        line(raw, Point(center.x, get<0>(tup)), Point(center.x, get<1>(tup)), Scalar(0, 255, 0), 3, LINE_8);
    }
    // display output image
    imgWin(raw, "res");
    waitKey(0);
    return 0;
}

// otsu
int otsu(Mat& img)
{
    ////// 超绿灰度分割
    Mat channel[3];
    split(img, channel);
    channel[1] = 2 * channel[1] - channel[0] - channel[2];
    return threshold(channel[1], img, 0, 255, THRESH_OTSU);
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

tuple<int, int> getPlantHeight(const Point2f& p, const Mat& img, int checkDis, int lowestTime)
{
    int highest = 0;
    int lowest = 0;
    int x = p.x;
    int y = p.y;
    ;    // get highest
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


