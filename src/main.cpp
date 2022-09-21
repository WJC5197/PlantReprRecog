#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <time.h> 

using namespace std;
using namespace cv;

int main()
{
    VideoCapture cap;
    cap.open(2);
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    cout<<"success open"<<endl;
    char pic_Name[128] = {};     			//照片名称

    if(!cap.isOpened())
    {
        cout << "The camera open failed!" << endl;
    }

    Mat frame;
 
    while(1)
    {
        cap >> frame;
        if(frame.empty())
            break;
        imshow("camera", frame);
        
	    time_t nowTime;
	    tm* now;

        if(waitKey(30)  == 'q') 		//按下q键进行拍照
        {
            time(&nowTime);				//获取系统当前时间戳
            now = localtime(&nowTime);	//将时间戳转化为时间结构体

            sprintf(pic_Name,"photo/%d-%d-%d %d:%d:%d.jpg",now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, 
            now->tm_hour+8, now->tm_min, now->tm_sec);
            imwrite(pic_Name, frame);	//将Mat数据写入文件
        }
    }
}
