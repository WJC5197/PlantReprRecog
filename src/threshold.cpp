#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <tuple>

#include "utility.hpp"

using namespace cv;
using namespace std;
//global variable
string window_name = "camera";
int screen_ratio;
int screen_width_max = 600;
chrono::microseconds thread1_duration;
chrono::microseconds thread2_duration;

// thresholds
int green_h_low_th = 45;
int green_s_low_th = 45;
int green_v_low_th = 0;
int green_h_high_th = 75;
int green_s_high_th = 150;
int green_v_high_th = 255;

// red is separate,need special treatment
int upper_red_h_low_th = 0;
int lower_red_h_low_th = 170;
int red_s_low_th = 15;
int red_v_low_th = 0;

int upper_red_h_high_th = 10;
int lower_red_h_high_th = 179;
int red_s_high_th = 255;
int red_v_high_th = 255;

Mat convolution_kernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
vector<Vec4i> hierarchy; // findContours
vector<vector<Point>> contours;

// (H-0~180;S-0~255;V-0~255)
class red_ball_extract
{
public:
	chrono::time_point<chrono::steady_clock> start_time;
	chrono::time_point<chrono::steady_clock> end_time;
	Mat& hsv; // must be hsv image
	red_ball_extract(Mat& hsv_):hsv(hsv_){}
	void operator()()
	{
		start_time = chrono::steady_clock::now();
		Mat upper_red = hsv;
		Mat lower_red = hsv;
		inRange(upper_red,
				Scalar(upper_red_h_low_th,red_s_low_th,red_s_low_th),
				Scalar(upper_red_h_high_th,red_s_high_th,red_v_high_th),
				upper_red);
		inRange(lower_red,
				Scalar(lower_red_h_low_th,red_s_low_th,red_s_low_th),
				Scalar(lower_red_h_high_th,red_s_high_th,red_v_high_th),
				lower_red);
		addWeighted(upper_red, 1.0, lower_red, 1.0, 0.0, hsv);
		morphologyEx(hsv, hsv, MORPH_OPEN, convolution_kernel);
        morphologyEx(hsv, hsv, MORPH_CLOSE, convolution_kernel);
		Canny(hsv, hsv, 20, 80);
		findContours(hsv,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_NONE);
		end_time = chrono::steady_clock::now();
		thread1_duration = chrono::duration_cast<chrono::microseconds>(end_time-start_time);
	}
};

class plant_extract
{
public:
	chrono::time_point<chrono::steady_clock> start_time;
	chrono::time_point<chrono::steady_clock> end_time;
	Mat& hsv;
	int upperMostPos;
	int downMostPos;
	plant_extract(Mat& hsv_):hsv(hsv_){}
	tuple<int,int> operator()()
	{
		start_time = chrono::steady_clock::now();
		int cols = hsv.cols;
		int rows = hsv.rows;
		inRange(hsv,
				Scalar(green_h_low_th,green_s_low_th,green_s_low_th),
				Scalar(green_h_high_th,green_s_high_th,green_v_high_th),
				hsv);
		morphologyEx(hsv, hsv, MORPH_OPEN, convolution_kernel);
        morphologyEx(hsv, hsv, MORPH_CLOSE, convolution_kernel);
		for(int i=0;i<rows;i++)
		{
			for(int j=0;j<cols;j++)
			{
				if(hsv.at<uchar>(i,j)==255)
				{
					upperMostPos = j;
					break;
				}
			}
		}
		for(int i=rows-1;i>0;i--)
		{
			for(int j=cols-1;j>0;j--)
			{
				if(hsv.at<uchar>(i,j)==255)
				{
					downMostPos = j;
				}
			}
		}
		end_time = chrono::steady_clock::now();
		thread2_duration = chrono::duration_cast<chrono::microseconds>(end_time-start_time);
		return tuple<int,int>(upperMostPos,downMostPos);
	}
};

void circle_recog(vector<vector<Point>> &contours,vector<vector<Point>> &save_contours,int &i,Mat &frame,bool &anchor,double &roundness)
{
	// filter the small contours
	double epilson = 0.01 * arcLength(contours[i],false);
	save_contours = contours;
	approxPolyDP(save_contours[i],contours[i],epilson ,true);
	//筛除面积很小的轮廓
	double area = contourArea(contours[i], true);
	roundness = -1;
	if (area<750)
	{
		contours[i].clear();
		return;
	}
	auto corners = contours[i].size();
	auto moment = moments(contours[i]);
	double x = moment.m10/moment.m00;
	double y = moment.m01/moment.m00;
	double x_left = contours[i][0].x;
	double x_right =contours[i][0].x ;
	double y_up = contours[i][0].y;
	double y_down = contours[i][0].y;
	for (int j=0;j<contours[i].size();j++)
	{
		if (contours[i][j].x<x_left)
		{
			x_left = contours[i][j].x;
		}
		else if(contours[i][j].x>x_right)
		{
			x_right = contours[i][j].x;
		}
		else if (contours[i][j].y<y_up)
		{
			y_up = contours[i][j].y;
		}
		else if (contours[i][j].y>y_down)
		{
			y_down = contours[i][j].y;
		}
	}
	if(corners>=15) // judge whether the contour is a circle
	{
		roundness = epilson / area;
		anchor = 1;
		circle(frame, Point2d(x, y), 3, Scalar(0, 255, 0), 3);
		rectangle(frame,Rect2d(x_left,y_up,x_right-x_left,y_down-y_up),Scalar(0,0,255),2);
		//通过圆度来判断是否为圆形
		//圆度的计算公式=周长^2/面积
	}
}
int main(int argc, char* argv[])
{
    // namedWindow(window_capture_name);
    // namedWindow(window_detection_name);
    // // Trackbars to set thresholds for HSV values
    // createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
    // createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
    // createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
    // createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
    // createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
    // createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
    // Mat frame, frame_HSV, frame_threshold;
    // frame = imread(file_name);
    // while (true) {
    //     // Convert from BGR to HSV colorspace
    //     cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
    //     // Detect the object based on HSV Range Values
    //     inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold);
    //     // Show the frames
    //     imshow(window_capture_name, frame);
    //     imshow(window_detection_name, frame_threshold);
    //     char key = (char) waitKey(30);
    //     if (key == 'q' || key == 27)
    //     {
    //         break;
    //     }
    // }
    // return 0;
    Mat hsv;
    Mat rgb = imread("../img/rawPlant.jpg");
    cvtColor(rgb,hsv,COLOR_BGR2HSV);
    plant_extract pe(hsv);
    pe();
    imwrite("../img/plant_extract.jpg",hsv);
    return 0;
}