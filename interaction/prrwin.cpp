#include "prrwin.h"
#include "./ui_prrwin.h"

#include "qtutility.hpp"
#include "../src/utility.hpp"

PRRWin::PRRWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PRRWin)
{
    ui->setupUi(this);
    // ui map
        // btn
        detectBtn = ui->detect;
        videoBtn = ui->video;
        calibrateBtn = ui->calibrate;
        // camera
        imgView = ui->imgView;
        videoView = ui->videoView;
        cameraList = ui->cameraList;
        cameraOpe = ui->cameraOpe;
        // input
        widthSet = ui->widthSet;
        heightSet = ui->heightSet;
        fpsSet = ui->fpsSet;
        // terminal ouput
        terminal = ui->terminal;
    // connect
    connect(detectBtn, SIGNAL(clicked()), this, SLOT(onDetectClicked()));
    connect(videoBtn, SIGNAL(clicked()), this, SLOT(onVideoClicked()));
    connect(calibrateBtn, SIGNAL(clicked()), this, SLOT(onDetectClicked()));

    // init
    auto items = getCameraList();
    for (const auto& item : items) {
        cameraList->addItem(QString::fromStdString(item));
    }
}

PRRWin::~PRRWin()
{
    delete ui;
}

void PRRWin::cameraInit()
{
//    camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
//    camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
//    camera.set(cv::CAP_PROP_FPS, fps);
}

void PRRWin::displayImg()
{
    cv::Mat img = cv::imread("E:/Bravelearn/Logo/mainlogo.png");
}

void PRRWin::displayVideo()
{

}

void PRRWin::onDetectClicked()
{
    cv::Mat img = cv::imread("D:/_Proj/CC/PlantReprRecog/img/1.jpg");
    std::cout << opencvDataType2Str(img.type()) << std::endl;
    cvtColor(img, img, cv::COLOR_BGR2RGB);

    ui->imgView->setPixmap(
        QPixmap::fromImage(
           QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)
        )
    );
    cv::waitKey();
}


void PRRWin::onWidthSetEditingFinished()
{
    QString widthStr = widthSet->text();
    if(!isIntInRange(widthStr, 640, 2592))
    {
        return;
    }
    camera.set(cv::CAP_PROP_FRAME_WIDTH, widthStr.toInt());
}


void PRRWin::onHeightSetEditingFinished()
{
    QString heightStr = heightSet->text();
    if(!isIntInRange(heightStr, 480, 1944))
    {
        return;
    }
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, heightStr.toInt());
}


void PRRWin::onfpsSetEditingFinished()
{
    QString fpsStr = fpsSet->text();
    if(!isIntInRange(fpsStr, 5, 30))
    {
        return;
    }
    camera.set(cv::CAP_PROP_FPS, fpsStr.toInt());
}

void PRRWin::onVideoClicked()
{

}


void PRRWin::updateCameraDevice(QAction *action)
{
    setCamera(qvariant_cast<QCameraDevice>(action->data()));
}


void PRRWin::updateCameras()
{
    cameraList->clear();
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras) {
        QAction *videoDeviceAction = new QAction(cameraDevice.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
            videoDeviceAction->setChecked(true);

        cameraList->addAction(videoDeviceAction);
    }
}

