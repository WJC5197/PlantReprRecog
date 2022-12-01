//test.cpp
#include "../src/stdc++.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;
template <typename T>
class work
{
public:
    work(T& workload_):workload(workload_){}
    template<typename... Args>
    void operator()(void(*func)(T&,Args...), Args... args)
    {
        chrono::time_point<chrono::steady_clock> start_time = chrono::steady_clock::now();
        // Mat upper_red = hsv;
        // Mat lower_red = hsv;
        // inRange(upper_red,
        // 		Scalar(upper_red_h_low_th,red_s_low_th,red_s_low_th),
        // 		Scalar(upper_red_h_high_th,red_s_high_th,red_v_high_th),
        // 		upper_red);
        // inRange(lower_red,
        // 		Scalar(lower_red_h_low_th,red_s_low_th,red_s_low_th),
        // 		Scalar(lower_red_h_high_th,red_s_high_th,red_v_high_th),
        // 		lower_red);
        // addWeighted(upper_red, 1.0, lower_red, 1.0, 0.0, hsv);
        // morphologyEx(hsv, hsv, MORPH_OPEN, convolution_kernel);
        // morphologyEx(hsv, hsv, MORPH_CLOSE, convolution_kernel);
        // Canny(hsv, hsv, 20, 80);
        // findContours(hsv,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_NONE);
        func(this->workload,args...);
        duration = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now()-start_time);
    }
    double cost()
    {
        return duration.count();
    }
private:
    chrono::microseconds duration;
    T& workload;
};
void add_sum(int& sum, vector<int>addend)
{
    for(int i=0;i<addend.size();i++)
    {
        sum+=addend[i];
    }
}
void add_one(int& sum)
{
    sum++;
}
int main(int argc, const char * argv[]) {

    cv::Mat image= cv::imread("../img/seedling.png");
    if (!image.data) {
        std::cout << "Image file not found\n";
        return 1;
    }

    //Prepare the image for findContours
    cv::cvtColor(image, image, COLOR_BGR2GRAY);
    threshold(image, image, 0, 255, THRESH_OTSU);
    //Find the contours. Use the contourOutput Mat so the original image doesn't get overwritten
    std::vector<std::vector<cv::Point> > contours;
    cv::Mat contourOutput = image.clone();
    cv::findContours( contourOutput, contours, RETR_LIST, CHAIN_APPROX_NONE );

    //Draw the contours
    cv::Mat contourImage(image.size(), CV_8UC3, cv::Scalar(0,0,0));
    cv::Scalar colors[3];
    colors[0] = cv::Scalar(255, 0, 0);
    colors[1] = cv::Scalar(0, 255, 0);
    colors[2] = cv::Scalar(0, 0, 255);
    for (size_t idx = 0; idx < contours.size(); idx++) {
        cv::drawContours(contourImage, contours, idx, colors[idx % 3]);
    }

    namedWindow("Contours",0);
	resizeWindow("Contours", 1024, 720);
    cv::imshow("Contours", image);
    cv::waitKey(0);

    return 0;
}


// #include <opencv2/imgproc/imgproc.hpp>
// #include <opencv2/highgui/highgui.hpp>
 
// #include <iostream>
 
// using namespace cv;
// using namespace std;
 
// Vec3b RandomColor(int value);  //生成随机颜色函数
 
// int main( int argc, char* argv[] )
// {
// 	Mat image=imread(argv[1]);    //载入RGB彩色图像
// 	imshow("Source Image",image);
 
// 	//灰度化，滤波，Canny边缘检测
// 	Mat imageGray;
// 	cvtColor(image,imageGray,COLOR_RGB2GRAY);//灰度转换
// 	GaussianBlur(imageGray,imageGray,Size(5,5),2);   //高斯滤波
// 	imshow("Gray Image",imageGray); 
// 	Canny(imageGray,imageGray,80,150);  
// 	imshow("Canny Image",imageGray);
 
// 	//查找轮廓
// 	vector<vector<Point>> contours;  
// 	vector<Vec4i> hierarchy;  
// 	findContours(imageGray,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point());  
// 	Mat imageContours=Mat::zeros(image.size(),CV_8UC1);  //轮廓	
// 	Mat marks(image.size(),CV_32S);   //Opencv分水岭第二个矩阵参数
// 	marks=Scalar::all(0);
// 	int index = 0;
// 	int compCount = 0;
// 	for( ; index >= 0; index = hierarchy[index][0], compCount++ ) 
// 	{
// 		//对marks进行标记，对不同区域的轮廓进行编号，相当于设置注水点，有多少轮廓，就有多少注水点
// 		drawContours(marks, contours, index, Scalar::all(compCount+1), 1, 8, hierarchy);
// 		drawContours(imageContours,contours,index,Scalar(255),1,8,hierarchy);
// 	}
 
// 	//我们来看一下传入的矩阵marks里是什么东西
// 	Mat marksShows;
// 	convertScaleAbs(marks,marksShows);
//     namedWindow("轮廓",0);
// 	resizeWindow("轮廓", 1024, 720);
// 	imshow("轮廓",imageContours);
// 	watershed(image,marks);
 
// 	//我们再来看一下分水岭算法之后的矩阵marks里是什么东西
// 	Mat afterWatershed;
// 	convertScaleAbs(marks,afterWatershed);
//     namedWindow("After Watershed",0);
// 	resizeWindow("After Watershed", 1024, 720);
// 	imshow("After Watershed",afterWatershed);
 
// 	//对每一个区域进行颜色填充
// 	Mat PerspectiveImage=Mat::zeros(image.size(),CV_8UC3);
// 	for(int i=0;i<marks.rows;i++)
// 	{
// 		for(int j=0;j<marks.cols;j++)
// 		{
// 			int index=marks.at<int>(i,j);
// 			if(marks.at<int>(i,j)==-1)
// 			{
// 				PerspectiveImage.at<Vec3b>(i,j)=Vec3b(255,255,255);
// 			}			 
// 			else
// 			{
// 				PerspectiveImage.at<Vec3b>(i,j) =RandomColor(index);
// 			}
// 		}
// 	}
// 	imshow("After ColorFill",PerspectiveImage);
 
// 	//分割并填充颜色的结果跟原始图像融合
// 	Mat wshed;
// 	addWeighted(image,0.4,PerspectiveImage,0.6,0,wshed);
// 	imshow("AddWeighted Image",wshed);
 
// 	waitKey();
// }
 
// Vec3b RandomColor(int value)
// {
// 	value=value%255;  //生成0~255的随机数
// 	RNG rng;
// 	int aa=rng.uniform(0,value);
// 	int bb=rng.uniform(0,value);
// 	int cc=rng.uniform(0,value);
// 	return Vec3b(aa,bb,cc);
// }
