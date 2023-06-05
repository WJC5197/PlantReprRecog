#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;
// Defining the dimensions of checkBoard
// 定义棋盘格的尺寸
fs::path curPath = fs::current_path().generic_string();
fs::path calibImgPath = (curPath / fs::path("img/calib")).generic_string();

int checkBoard[2]{6, 9};

//标定
void calibrateCamera(vector<Mat>, int, int, int, int, Mat&, Mat&);

//提取角点
vector<Mat> findChessCorner(vector<Mat>, Size, vector<vector<Point2f>>&);

//畸变矫正
void undistortImage(Mat, Mat, Mat, Mat&);

int main()
{
	vector<Mat> imgVec;
	for (const auto& entry : fs::directory_iterator(calibImgPath))
		imgVec.push_back(imread(entry.path().string()));

	Mat cameraMat;
	Mat distCoeffs;
	calibrateCamera(imgVec, 9, 5, 15, 15, cameraMat, distCoeffs);

	Mat newImg;
	undistortImage(imgVec[0], cameraMat, distCoeffs, newImg);

	waitKey(0);
}

void calibrateCamera(vector<Mat> imageVector, int rowCornersCount, int colCornersCount, int gridWidth, int gridHeight, Mat& cameraMatrix, Mat& distCoeffs)
{
	vector<Mat> imageCalibrateVector;
	Size board_size = Size(rowCornersCount, colCornersCount);
	vector<vector<Point2f>> image_points_seq;
	vector<Mat> imageCornerVector = findChessCorner(imageVector, board_size, image_points_seq);

	for (int imageIndex = 0; imageIndex < imageCornerVector.size(); imageIndex++)
	{
		Mat imageCorner = imageCornerVector[imageIndex];
		Size image_size;
		image_size.width = imageCorner.cols;
		image_size.height = imageCorner.rows;

		if (!imageCorner.data)
			cout << "未提取角点,无法标定!" << endl;
		//以下是摄像机标定
		cout << "开始标定...";
		/*棋盘三维信息*/
		Size square_size = Size(gridWidth, gridHeight);  /* 实际测量得到的标定板上每个棋盘格的大小 */
		vector<vector<Point3f>> object_points; /* 保存标定板上角点的三维坐标 */
		/*内外参数*/
		cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* 摄像机内参数矩阵 */
		vector<int> point_counts;  // 每幅图像中角点的数量
		distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0)); /* 摄像机的5个畸变系数：k1,k2,p1,p2,k3 */
		vector<Mat> tvecsMat;  /* 每幅图像的平移向量 */
		vector<Mat> rvecsMat; /* 每幅图像的旋转向量 */
		/* 初始化标定板上角点的三维坐标 */
		int i, j, t;
		for (t = 0; t < imageVector.size(); t++)
		{
			vector<Point3f> tempPointSet;
			for (i = 0; i < board_size.height; i++)
			{
				for (j = 0; j < board_size.width; j++)
				{
					Point3f realPoint;
					/* 假设标定板放在世界坐标系中z=0的平面上 */
					realPoint.x = i * square_size.width;
					realPoint.y = j * square_size.height;
					realPoint.z = 0;
					tempPointSet.push_back(realPoint);
				}
			}
			object_points.push_back(tempPointSet);
		}
		/* 初始化每幅图像中的角点数量，假定每幅图像中都可以看到完整的标定板 */
		for (i = 0; i < imageVector.size(); i++)
		{
			point_counts.push_back(board_size.width * board_size.height);
		}
		/* 开始标定 */
		calibrateCamera(object_points, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);
		cout << "标定完成！\n";
		/*保存内参和畸变系数，以便后面直接矫正*/
		//ofstream fout("caliberation_result.txt");//保存标定结果的文件
		cout << "相机内参数矩阵：" << endl;
		cout << cameraMatrix << endl << endl;
		cout << "畸变系数：\n";
		cout << distCoeffs << endl << endl << endl;

		//cout.close();
	}

}

//提取角点
vector<Mat> findChessCorner(vector<Mat> imageVector, Size board_size, vector<vector<Point2f>>& image_points_seq)
{
	vector<Mat> imageCornerVector;
	int imageCount = imageVector.size();
	Size image_size;  /* 图像的尺寸 */
	vector<Point2f> image_points_buf;  /* 缓存每幅图像上检测到的角点 */

	for (int imageIndex = 0; imageIndex < imageVector.size(); imageIndex++)
	{
		Mat imageInput = imageVector[imageIndex];

		image_size.width = imageInput.cols;
		image_size.height = imageInput.rows;

		Mat view_gray;
		/* 提取角点 */
		if (0 == findChessboardCorners(imageInput, board_size, image_points_buf))
		{
			cout << "找不到角点!\n"; //找不到角点
			exit(1);
		}
		else
		{
			cout << "角点优化中..." << endl;
			cvtColor(imageInput, view_gray, cv::COLOR_RGB2GRAY);
			/* 亚像素精确化 */
			find4QuadCornerSubpix(view_gray, image_points_buf, Size(5, 5)); //对粗提取的角点进行精确化
			//cornerSubPix(view_gray,image_points_buf,Size(5,5),Size(-1,-1),TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,30,0.1));
			image_points_seq.push_back(image_points_buf);  //保存亚像素角点
			/* 在图像上显示角点位置 */
			drawChessboardCorners(view_gray, board_size, image_points_buf, false); //用于在图片中标记角点

			namedWindow("cornerWindow", 0);
			imshow("cornerWindow", view_gray);
			imageCornerVector.push_back(view_gray);
			waitKey(2000);
			destroyWindow("cornerWindow");
			cout << "角点提取完成！\n";
		}
	}

	return imageCornerVector;
}


//畸变矫正
void undistortImage(Mat image, Mat cameraMatrix, Mat distCoeffs, Mat& newImage)
{
	//*读取之前标定好的数据直接矫正*/
	Mat newimage = image.clone();
	undistort(image, newImage, cameraMatrix, distCoeffs);
	namedWindow("UndistortWindow", 0);
	imshow("UndistortWindow", newImage);

	waitKey(2000000);//停半秒
}
