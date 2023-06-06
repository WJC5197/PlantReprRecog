#include "prrwin.h"
#include "./ui_prrwin.h"
#include "../requires.h"
#include "../../util/utility.hpp"

void PRRWin::frameProcess(cv::Mat &frame)
{
    // thresSeg(frame);
    findCenters(frame, centers);
    // if centers are too dense

    for (const auto &center : centers)
    {
        cout << "||>> center:" << center.x << "," << center.y << endl;
        // circle(raw, Point(center.x, center.y), 15, Scalar(255,0,0), -1);
        auto tup = getPlantHeight(center, frame, 50, 400);
        plantHeights.push_back(tup);
        // line(raw, Point(center.x, get<0>(tup)), Point(center.x, get<1>(tup)), Scalar(0, 255, 0), 3, LINE_8);
    }
}

double PRRWin::mapCycleToHeight(double cycle)
{
    return cycle / 400 * 1;
}

void PRRWin::imgProcess(int id, const QImage &image)
{
    qtFrame = image.convertToFormat(QImage::Format_RGBA8888);
    cvFrame = cv::Mat(image.height(), image.width(), CV_8UC4, (uchar *)image.bits(), image.bytesPerLine()).clone();
    qDebug() << "|>imgProcess: " << opencvDataType2Str(cvFrame.type()).c_str();
    cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGRA2RGB);
    //    cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGR2RGB);
    //    imwrite("..\\img\\test.jpg", cvFrame);
    //     cvFrame = QtOcv::image2Mat(image);
}