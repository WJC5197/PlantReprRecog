// rtt means "real time test"
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "utility.hpp"

using namespace cv;
using namespace std;

int screen_ratio;
int screen_width_max = 600;

// (H-0~180;S-0~255;V-0~255)
int green_h_low_th = 45;
int green_s_low_th = 45;
int green_v_low_th = 0;
int green_h_high_th = 75;
int green_s_high_th = 150;
int green_v_high_th = 255;

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
chrono::microseconds thread1_duration;
chrono::microseconds thread2_duration;

int upperMostPos;
int downMostPos;
double roundness;
double ball_center_x;
double ball_center_y;

struct plant_extract
{
	Mat& hsv;
	chrono::time_point<chrono::steady_clock> start_time;
	chrono::time_point<chrono::steady_clock> end_time;
	plant_extract(Mat& hsv_):hsv(hsv_){}
	void operator()()
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
					downMostPos = i;
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
					upperMostPos = i;
					break;
				}
			}
		}
		end_time = chrono::steady_clock::now();
		thread1_duration = chrono::duration_cast<chrono::microseconds>(end_time-start_time);
	}
};
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
		thread2_duration = chrono::duration_cast<chrono::microseconds>(end_time-start_time);
	}
};

void circle_recog(vector<Point> &contour)
{
	double epilson = 0.1 * arcLength(contour,true);
	auto save_contour = contour;
	approxPolyDP(contour,save_contour,epilson ,true);
	//筛除面积很小的轮廓
	double area = abs(contourArea(save_contour, true));
	roundness = -1;
	if (area<10000)
	{
		contour.clear();
		return;
	}
	auto corners = contour.size();
	roundness = epilson / area; // caculate roundness
	cout<<"|contour:{area:"<<area<<",corners:"<<corners<<",roundness:"<<roundness<<"}"<<endl;
	if (corners >= 8 && roundness <= 0.707)
	{
		auto br = boundingRect(contour);
		ball_center_x = br.x+br.width/2;
		ball_center_y = br.y+br.height/2;
		return;
	}
	else
	{
		contour.clear();
	}

}

int main()
{
	roundness = -1;
    Mat greenRaw = imread("../img/rawPlant.jpg");
	Mat redRaw = imread("../img/rawPlant.jpg");
    // cout << openCVType2str(greenRaw.type()) <<endl;
    Mat hsv;
	Mat redHsv;
    cvtColor(greenRaw,hsv,COLOR_BGR2HSV);
	cvtColor(redRaw,redHsv,COLOR_BGR2HSV);

    auto pe=plant_extract(hsv);
	auto re=red_ball_extract(redHsv);
	// re();

	thread plant_extract_thread(pe);
	thread red_ball_extract(re);
    plant_extract_thread.join();
	red_ball_extract.join();
	
	cout<<"|red channel contours size:"<<contours.size()<<endl;
	for (int i=0;i<contours.size();)
	{
		circle_recog(contours[i]);
		if(contours[i].empty())
		{
			contours.erase(contours.begin()+i);
			// cout<<"empty!"<<endl;
		}
		else ++i;
	}
	cout<<"|after process red channel contours size:"<<contours.size()<<endl;
	// drawing
	Mat redDrawing = Mat::zeros( redHsv.size(), CV_8UC3 );
	for( size_t i = 0; i< contours.size(); i++ )
    {
		drawContours(redDrawing, contours, (int)i, Scalar(255,0,255), 2, LINE_8, hierarchy, 0);
    }

	// resize(hsv, hsv, Size(600, 400), INTER_LINEAR);
	resize(redDrawing, redDrawing, Size(400, 400*(redDrawing.rows/redDrawing.cols)), INTER_LINEAR);
	resize(hsv, hsv, Size(400, 400*(hsv.rows/hsv.cols)), INTER_LINEAR);
	resize(greenRaw, greenRaw, Size(400, 400*(greenRaw.rows/greenRaw.cols)), INTER_LINEAR);

    cout<<"|upper:"<<upperMostPos<<endl;
	cout<<"|down:"<<downMostPos<<endl;
	cout<<"|ball_center:{x:"<<ball_center_x<<",y:"<<ball_center_y<<"}"<<endl;
    imshow("ball",redDrawing);
    imshow("plant",hsv);
	imshow("raw",greenRaw);
	cout << "|thread1:"<< thread1_duration.count()<<"us"<<endl;
	cout << "|thread2:"<< thread2_duration.count()<<"us"<<endl;
    waitKey(0);
    return 0;
}
