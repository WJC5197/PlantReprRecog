// rtt means "real time test"
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "stdc++.h"
#include "utility.hpp"
#include "phm.hpp"

using namespace cv;
using namespace std;

string plant_seg_win = "height measure";
int win_width = 640;
int win_height = 480;
//double areaThres = 60000;

void plant_recog(Mat& workload, vector<Vec4i>& hierarchy, vector<vector<Point>>& filtrated_contours)
{
	////// 超绿灰度分割
	thresSeg(workload);
	//Mat channel[3];
	//split(workload, channel);

	//channel[1] = 2 * channel[1] - channel[0] - channel[2];

	//int otsu_thres = threshold(channel[1], workload, 0, 255, THRESH_OTSU);
	imgWin(workload, "otsu");
	vector<vector<Point>> contours;
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
			filtrated_contours.push_back(contours[i]);
	}
	cout << "|>filtrated contours' size:" << filtrated_contours.size() << endl;

	////// filtrate not plant contour
	double x_mean;
	double y_mean;
	Point highest;
	Point lowest;
	double y_min;
	workload = Mat(workload.size(), CV_8UC3);
	for (int i = 0; i < filtrated_contours.size(); i++)
	{
		auto minmax = minmax_element(filtrated_contours[i].begin(),
			filtrated_contours[i].end(),
			[](const Point& a, const Point& b)
			{
				return a.y < b.y;
			});
		cout << "|>the" << i << " elem:{y_min:" << minmax.first->y << ",y_max:" << minmax.second->y << "}" << endl;
		x_mean = accumulate(filtrated_contours[i].begin(), filtrated_contours[i].end(), 0, [](float sum, Point p) {return sum + p.x; }) / filtrated_contours[i].size();
		cout << "|>the" << i << " elem:{x_mean:" << x_mean << "}" << endl;
		highest.x = x_mean;
		highest.y = minmax.second->y;
		lowest.x = x_mean;
		lowest.y = minmax.first->y;
		drawContours(workload, filtrated_contours, (int)i, Scalar(0, 0, 0), 2, LINE_8, hierarchy, 0);
		line(workload, highest, lowest, Scalar(0, 0, 255), 2);
		putText(workload, to_string(highest.y - lowest.y) + "px", Point(x_mean, (highest.y + lowest.y) / 2), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 3);
	}
	////// draw filtrated contours
}

int main(int argc, char* argv[])
{
	////// read image
	Mat img = imread((fs::current_path() / fs::path("../img/camera.png")).generic_string());

	Mat otsuMat;
	cv::cvtColor(img,otsuMat,cv::COLOR_BGR2GRAY);
	cv::threshold(otsuMat, otsuMat, 0, 255, cv::THRESH_OTSU);
	otsuMat = ~otsuMat;
	cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));

    //cv::morphologyEx(otsuMat, otsuMat, cv::MORPH_CLOSE, dilateKernel);

	Mat raw = img;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point2f> centers;
	thresSeg(img);
	cout << "highest: " << lightRegionMeanMaxHeight(img) << "; Height" << img.cols << endl;
	cout << "lightness is:" << getLightness(img) << endl;
	cout << opencvDataType2Str(img.type()) << endl;
	// count the cost time of the below process
	std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();
	findCenters(img, centers);
	imgWin(img, "otsu");
	// draw centers on output image
	for (const auto& center : centers)
	{
		cout << "||>> center:" << center.x << "," << center.y << endl;
		circle(raw, Point(center.x, center.y), 15, Scalar(255, 0, 0), -1);
		auto tup = getPlantHeight(center, img, 50, 400);
		line(raw, Point(center.x, get<0>(tup)), Point(center.x, get<1>(tup)), Scalar(0, 255, 0), 3, LINE_8);
	}
	std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now();
	cout << "|>time cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << endl;
	imgWin(raw, "res");
	waitKey(0);
	//VideoCapture capture(1);
	// camera args
	//capture.set(CAP_PROP_FRAME_WIDTH, 1920);      // 宽度
	//capture.set(CAP_PROP_FRAME_HEIGHT, 1080);    // 高度
	//capture.set(CAP_PROP_FPS, 15);               // 帧率
	//while (true)
	//{
		//capture >> frame;
		//plant_recog_work(plant_recog,hierarchy,contours);
		////cout <<"|>time cost:"<< plant_recog_work.timeCost() << endl;
		//namedWindow(plant_seg_win,0);
		//resizeWindow(plant_seg_win, win_width, win_height);
		//imshow(plant_seg_win, img);
		//contours.clear();
		//hierarchy.clear();
		//waitKey(0);	//延时30
	//}
	//Mat img = imread("../img/1.jpg");
	//Mat res = img; // intermediate image


	/*if (img.empty())
	{
		cout <<"Error: Could not load image" <<endl;
		return 0;
	}*/


	// threshold(channel[1], thresSeg, otsu_thres, 255, THRESH_BINARY);
// imshow window
// namedWindow(plant_seg_win,0);
// resizeWindow(plant_seg_win, win_width, win_height);
// imshow(plant_seg_win,thresSeg);
// waitKey(100000);

////// morphology operation

////// Canny edge detection
	// find threshold

		// mmean and gamma method
		// double mean = mean(thresSeg);
		// double gamma = 0.33;
		// double low_thres=(1-gamma)*mean;
		// double high_thres=(1+gamma)*mean;

		// median method
		//GaussianBlur(thresSeg, thresSeg, Size(3, 3), 0.5, 0.5);
		//int low_thres;
		//int high_thres;
		//get_canny_minmax_thres(thresSeg,low_thres,high_thres,0.33);

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
//thresSeg = thresSeg & canny;
// morphologyEx(thresSeg, thresSeg, MORPH_CLOSE, close_kernel);
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


////// filtrate not plant contour

////// imshow window
//namedWindow(plant_seg_win,0);
//resizeWindow(plant_seg_win, win_width, win_height);
//imshow(plant_seg_win,res);
//waitKey(0);
	return 0;
}
