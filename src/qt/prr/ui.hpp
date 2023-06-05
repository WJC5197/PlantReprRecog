#include "prrwin.h"
#include "./ui_prrwin.h"
#include "../requires.h"
#include "../imgset/imagesettings.h"

// Button Click
void PRRWin::onMeasureClicked()
{
#if _ORANGE_PI_
    serialInit();
#endif
    qStdOut << height << Qt::endl;
    initCamera();
    imgCapture->capture();
    qtDelay(2);
    qDebug() << cvFrame.empty() << Qt::endl;
    displayImgView();
#if _ORANGE_PI_
    phm();
#endif
}

void PRRWin::onVideoClicked()
{
    imgView->clear();
    captureSession.setVideoOutput(videoView);
    displayVideoView();
}

void PRRWin::onCalibrateClicked()
{
    imgCapture->captureToFile("/root/tmp.jpg");
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

void PRRWin::displayImgView(cv::Mat &mat)
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