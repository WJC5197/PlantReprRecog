#include "../imgset/imagesettings.h"
#include "../requires.h"
#include "./ui_prrwin.h"
#include "prrwin.h"

// Button Click
void PRRWin::onSingleClicked()
{
    double ascend = 20;
    double yInitLow, yInitHigh, yEndLow, yEndHigh;
    // imgCapture->capture();
    // read from an img
    cvFrame = cv::imread("../img/1.jpg");
    thresSeg(cvFrame, true);
    cv::Mat r = cv::Mat::zeros(cvFrame.size(), cvFrame.type());
    cv::Mat b = cv::Mat::zeros(cvFrame.size(), cvFrame.type());
    cv::Mat tmp = cv::Mat::zeros(cvFrame.size(), cvFrame.type());
    cv::merge(std::vector<cv::Mat>{b, cvFrame, r}, tmp);
    auto rects = getPlantRegion(tmp);
    filtrateRegion(rects);
    // find the rect with maximum area
    auto maxRect = std::max_element(rects.begin(), rects.end(), [](const cv::Rect &a, const cv::Rect &b)
                                    { return a.area() < b.area(); });
    yInitLow = maxRect->y;
    yInitHigh = maxRect->y + maxRect->height;
    rects = {*maxRect};
    qtFrame = QImage((uchar *)tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888);
    QPainter painter(&qtFrame);
    painter.setPen(QPen(Qt::green, 2, Qt::SolidLine));
    for (auto &rect : rects)
    {
        QRect qrect(rect.x, rect.y, rect.width, rect.height);
        painter.drawRect(qrect);
    }
    displayImgView();
    qtDelay(3);

    cvFrame = cv::imread("../img/2.jpg");
    thresSeg(cvFrame, true);
    r = cv::Mat::zeros(cvFrame.size(), cvFrame.type());
    b = cv::Mat::zeros(cvFrame.size(), cvFrame.type());
    tmp = cv::Mat::zeros(cvFrame.size(), cvFrame.type());
    cv::merge(std::vector<cv::Mat>{b, cvFrame, r}, tmp);
    rects = getPlantRegion(tmp);
    std::cout << "success!" << endl;
    filtrateRegion(rects);
    std::cout << "success!" << endl;
    // find the rect with maximum area
    maxRect = std::max_element(rects.begin(), rects.end(), [](const cv::Rect &a, const cv::Rect &b)
                               { return a.area() < b.area(); });
    std::cout << "success!" << endl;
    yEndLow = maxRect->y;
    yEndHigh = maxRect->y + maxRect->height;
    std::cout << yInitLow << " " << yInitHigh << " " << yEndLow << " " << yEndHigh << endl;
    // qtFrame = QImage((uchar *)tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888);
    QRect qrect(maxRect->x, maxRect->y, maxRect->width, maxRect->height);
    painter.drawRect(qrect);
    displayImgView();
    std::cout << "success!" << endl;
    qStdOut << "||>> PlantHeight is :" << ascend / abs(yEndLow - yInitLow) * abs(yInitHigh - yInitLow) << Qt::endl;
}

void PRRWin::onContinuousClicked()
{
    serialInit();
    qStdOut << height << Qt::endl;
    initCamera();
    imgCapture->capture();
    qtDelay(2);
    qDebug() << cvFrame.empty() << Qt::endl;
    displayImgView();
    phm();
}

void PRRWin::onVideoClicked()
{
    imgView->clear();
    captureSession.setVideoOutput(videoView);
    displayVideoView();
}

void PRRWin::onCalibrateClicked()
{
    // imgCapture->captureToFile("/root/tmp.jpg");
}

void PRRWin::onCleanClicked()
{
    ui->terminal->clear();
}

// View
void PRRWin::displayVideoView()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void PRRWin::displayImgView(const cv::Mat &mat)
{
    // cv::Mat tmp = cv::Mat::zeros(mat.size(), mat.type());
    // cv::cvtColor(mat, tmp, cv::COLOR_RGB2BGR);
    // qtFrame = QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888);
    cv::Mat tmp = mat.clone();
    cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
    qtFrame = QImage((uchar *)tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888);
    ui->stackedWidget->setCurrentIndex(1);
    imgView->setPixmap(QPixmap::fromImage(qtFrame.scaled(imgView->width(), imgView->height(), Qt::KeepAspectRatio)));
    imgView->setScaledContents(true);
}

void PRRWin::displayImgView()
{
    ui->stackedWidget->setCurrentIndex(1);
    imgView->setPixmap(QPixmap::fromImage(qtFrame.scaled(imgView->width(), imgView->height(), Qt::KeepAspectRatio)));
    imgView->setScaledContents(true);
}

void PRRWin::cfgImgSettings()
{
    ImageSettings settingsDialog(imgCapture.get());
    settingsDialog.setWindowFlags(settingsDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (settingsDialog.exec())
    {
        auto resolution = settingsDialog.applyImageSettings();
        width = get<0>(resolution);
        height = get<1>(resolution);
    }
}