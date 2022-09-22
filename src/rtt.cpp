#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "utility.hpp"

using namespace cv;
using namespace std;

// (H-0~180;S-0~255;V-0~255)
int green_h_low_th = 45;
int green_s_low_th = 45;
int green_v_low_th = 0;
int green_h_high_th = 75;
int green_s_high_th = 150;
int green_v_high_th = 255;
Mat convolution_kernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));

struct plant_extract
{
	Mat& hsv;
	plant_extract(Mat& hsv_):hsv(hsv_){}
	void operator()()
	{
		inRange(hsv,
				Scalar(green_h_low_th,green_s_low_th,green_s_low_th),
				Scalar(green_h_high_th,green_s_high_th,green_v_high_th),
				hsv);
        morphologyEx(hsv, hsv, MORPH_OPEN, convolution_kernel);
        morphologyEx(hsv, hsv, MORPH_CLOSE, convolution_kernel);
	}
};

void circle_recog(vector<vector<Point>> &contours,vector<vector<Point>> &save_contours,int &i,Mat &frame,bool &anchor,double &roundness)
{
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
	//circle(frame,(x,y),3,(0,0,255),-1);

	if (corners==3)
	{
		anchor = 1;
		circle(frame,Point2d(x,y),3,Scalar(255,0,0),3);
		rectangle(frame,Rect2d(x_left,y_up,x_right-x_left,y_down-y_up),Scalar(255,0,0),2);
	}
	else if (corners==4)
	{
		anchor = 1;
		circle(frame,Point2d(x,y),3,Scalar(0,0,255),3);
		rectangle(frame,Rect2d(x_left,y_up,x_right-x_left,y_down-y_up),Scalar(0,0,255),2);
	}
	else if (corners>=15)
	{
		roundness = epilson / area;
		anchor = 1;
		circle(frame, Point2d(x, y), 3, Scalar(0, 255, 0), 3);
		rectangle(frame,Rect2d(x_left,y_up,x_right-x_left,y_down-y_up),Scalar(0,0,255),2);
		//通过圆度来判断是否为圆形
		//圆度的计算公式=周长^2/面积
	}
}
int main()
{
    bool anchor=0;
	double roundness = -1;
	double area = 0;
	// 循环读取视频帧
	vector<vector<Point>> contours;
	vector<vector<Point>> save_contours;
	vector<Vec4i>hierarchy;

    Mat raw = imread("../img/rawPlant.jpg");
    cout << openCVType2str(raw.type()) <<endl;
    Mat hsv;
    cvtColor(raw,hsv,COLOR_BGR2HSV);
    auto pe=plant_extract(hsv);
    pe();
    // Canny(raw, raw, 20, 80);
    imshow("canny",hsv);
    waitKey(0);
    return 0;
}
// int main()
// {   
// 	// vector<Mat> channels;
//     // Mat rawPlant = imread("../img/rawPlant.jpg",IMREAD_COLOR);
//     // Mat output;
//     // cvtColor(rawPlant,rawPlant,COLOR_BGR2HSV);
//     // split(rawPlant,channels);
//     // int cols = rawPlant.cols;
//     // int rows = rawPlant.rows;
//     // cout << openCVType2str(rawPlant.type()) <<endl;
    
//     // vector<Mat> zero_channels;
//     // Mat zero_mat = Mat::zeros(720,720,CV_8UC3);
//     // cvtColor(zero_mat,zero_mat,COLOR_BGR2HSV);
//     // split(zero_mat,zero_channels);
//     // zero_channels[0] = 180;
//     // zero_channels[1] = 255; // Saturation
//     // zero_channels[2] = 255; // Value
//     // merge(zero_channels,zero_mat);
//     // Mat new_mat;
//     // cvtColor(zero_mat,new_mat,COLOR_BGR2HSV);
//     // imshow("zeros",new_mat);
//     // for(int i = 300;i<600;i++)
//     // {
//     //     for(int j=100;j<400;j++)
//     //     {
//     //         cout<<int(channels[0].at<uchar>(i,j))<<" ";
//     //     }
//     //     cout<<endl;
//     // }
//     // for(int i = 0;i<rows;i++)
//     // {
//     //     for(int j =0;j<cols;j++)
//     //     {
//     //         if(channels[0].at<uchar>(i,j)<get<0>(hue_green_threshold) && channels[0].at<uchar>(i,j)>get<1>(hue_green_threshold))
//     //         {
//     //             cout << "yes" <<endl;
//     //             channels[0].at<uchar>(i,j)=0;
//     //             channels[1].at<uchar>(i,j)=0;
//     //             channels[2].at<uchar>(i,j)=255;
//     //         }
//     //     }
//     // }
//     // merge(channels,output);
//     // imshow("Image",output);
//     // cout << output << endl; 
//     // Mat test=imread("../img/Lambda.jpg",IMREAD_COLOR);
//     // cout << "good!" <<endl;
//     // imshow("Image", test); // show the image
//     waitKey(0); // wait for a key pressed
//     return 1;
// }