// rtt means "real time test"
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "stdc++.h"
#include "utility.hpp"

using namespace cv;
using namespace std;

string plant_seg_win = "height measure";
int win_width = 640;
int win_height = 480;
double area_threshold = 100000;
RNG rng(12345);

void plant_recog(Mat& workload, vector<Vec4i> &hierarchy, vector<vector<Point>>& filtered_contours)
{
	////// 超绿灰度分割
	Mat channel[3];
	split(workload, channel);

	channel[1] = 2 * channel[1] - channel[0] - channel[2];

	int otsu_thres = threshold(channel[1], workload, 0, 255, THRESH_OTSU);
	vector<vector<Point>> contours;
	findContours(
		workload,
		contours,
		hierarchy,
		RETR_TREE,
		CHAIN_APPROX_SIMPLE
	);
	////// filtering contours by threshold area
	double area;
	for (int i = 0; i < contours.size(); i++)
	{
		area = contourArea(contours[i]);
		if (area > area_threshold)
			filtered_contours.push_back(contours[i]);
	}
	cout << "|>filtered contours' size:" << filtered_contours.size() << endl;
}

int main(int argc, char* argv[])
{	
	////// read image
	Mat img = imread("../img/1.jpg");
	Mat interm_img = img; // intermediate image
	Mat res = Mat(img.size(), CV_8UC3);
	Mat dilate_kernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
	Mat close_kernel = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
	auto size = img.size();
	
	vector<vector<Point>> filtered_contours;
	vector<Vec4i> hierarchy;

	if (img.empty())
	{
		cout <<"Error: Could not load image" <<endl;
		return 0;
	}
	
	work<Mat>plant_recog_work(interm_img);

	plant_recog_work(plant_recog,hierarchy,filtered_contours);
	cout << plant_recog_work.cost() << endl;
	// threshold(channel[1], otsu, otsu_thres, 255, THRESH_BINARY);
	// imshow window
	// namedWindow(plant_seg_win,0);
	// resizeWindow(plant_seg_win, win_width, win_height);
	// imshow(plant_seg_win,otsu);
	// waitKey(100000);

	////// morphology operation
	
	////// Canny edge detection
		// find threshold

			// mmean and gamma method
			// double mean = mean(otsu);
			// double gamma = 0.33;
			// double low_thres=(1-gamma)*mean;
			// double high_thres=(1+gamma)*mean;

			// median method
			//GaussianBlur(otsu, otsu, Size(3, 3), 0.5, 0.5);
			//int low_thres;
			//int high_thres;
			//get_canny_minmax_thres(otsu,low_thres,high_thres,0.33);
			
	// blur(img, interm_img, Size(5,5));
	// medianBlur(interm_img, interm_img);
	// for(int i=0;i<size.width;i++)
	// {
	// 	for(int j=0;j<size.height;j++)
	// 	{
	// 		low_thres=(low_thres>0.7*interm_img.at<int>(i,j))? 
	// 	}
	// }
	// low_thres = int(max(0,0.7*interm_img));
	// high_thres = int(min(255,1.3*interm_img));
	
	//Mat canny;
	//Canny(img, canny, 0.5*otsu_thres, otsu_thres);
	//// Canny(img, canny, low_thres, high_thres);
	//// morphologyEx(canny, canny, MORPH_DILATE, dilate_kernel);
	////canny = ~canny;
	//otsu = otsu & canny;
	// morphologyEx(otsu, otsu, MORPH_CLOSE, close_kernel);
	//namedWindow(plant_seg_win,0);
	//resizeWindow(plant_seg_win, win_width, win_height);
	//imshow(plant_seg_win,canny);
	//waitKey(0);


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

	//cout<<"|>raw contours' size:"<<contours.size()<<endl;
	//cout<<"|>hierarchy's size:"<<hierarchy.size()<<endl;
	//for(int i=0;i<hierarchy.size();i++)
	//{
	//	cout<<"|>hierarchy element "<<i<<":"<<hierarchy[i]<<endl;
	//}

	////// draw filtered contours
	for(size_t i = 0; i< filtered_contours.size();i++)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256));
        drawContours( res, filtered_contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
    }
	////// 计算contour横向的方差大小,滤除contour

	////// imshow window
	namedWindow(plant_seg_win,0);
	resizeWindow(plant_seg_win, win_width, win_height);
	imshow(plant_seg_win,res);
	waitKey(0);
	return 0;
}
