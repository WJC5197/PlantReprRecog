#include "prrwin.h"
#include "./ui_prrwin.h"

#include "qtutility.hpp"
#include "../src/utility.hpp"

#include "../src/communicate.hpp"

#include "../src/phm.hpp"

PRRWin::PRRWin(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::PRRWin)
{
    ui->setupUi(this);
    // ui map
    // btn
    detectBtn = ui->detect;
    videoBtn = ui->video;
    calibrateBtn = ui->calibrate;
    cleanBtn = ui->clean;
    // menu
    cameraOpen = ui->cameraOpen;
    cameraClose = ui->cameraClose;
    cameraSetting = ui->cameraSetting;
    camerasGroup = new QActionGroup(this);
    camerasGroup->setExclusive(true);

    // camera
    imgView = ui->imgView;
    videoView = ui->videoView;
    cameraList = ui->cameraList;

    // terminal
    terminal = new Terminal(ui->terminal);
    terminal->open(QIODevice::WriteOnly);
    qStdOut.setDevice(terminal);

    // stack widget
    stackedView = ui->stackedWidget;

    updateCameraList();

    // connect
    // menu
    connect(cameraOpen, SIGNAL(triggered()), this, SLOT(startCamera()));
    connect(cameraClose, SIGNAL(triggered()), this, SLOT(closeCamera()));
    connect(ui->cameraSetting, SIGNAL(triggered()), this, SLOT(cfgImgSettings()));
    // btn
    connect(detectBtn, SIGNAL(clicked()), this, SLOT(onMeasureClicked()));
    connect(videoBtn, SIGNAL(clicked()), this, SLOT(onVideoClicked()));
    connect(calibrateBtn, SIGNAL(clicked()), this, SLOT(onCalibrateClicked()));
    connect(cleanBtn, SIGNAL(clicked()), this, SLOT(onCleanClicked()));
    // camera
    connect(camerasGroup, SIGNAL(triggered(QAction *)), this, SLOT(updateCamera(QAction *)));
    connect(&devices, SIGNAL(videoInputsChanged()), this, SLOT(updateCameraList()));

    setCamera(QMediaDevices::defaultVideoInput());
}

PRRWin::~PRRWin()
{
    delete ui;
}

#if _ORANGE_PI_
void PRRWin::serialInit()
{
    qStdOut << "|> plant height measure init..." << Qt::endl;
    // init serial
    if (openSerial(serial0, "/dev/ttyS0", 115200) == -1)
    {
        qStdOut << "|> open serial error.";
        return;
    }
    qStdOut << "|> open serial success." << Qt::endl;
}

void PRRWin::phmControl()
{
    do
    {
        imgCapture->capture();
        otsu(cvFrame);
        plantImgs.push(cvFrame);
        lightnessPercent = getLightness(cvFrame) / 255;
        serialSend(serial0, "speed", 400);
        qtDelay(0.3);
        serialSend(serial0, "distance", 400);
    } while (lightnessPercent >= lightnessThres);
    phmFinished = true;
}

void PRRWin::phmComputation()
{
    cv::Mat img;
    while (phmFinished == true && plantImgs.size() == 0)
    {
        img = plantImgs.front();
        plantImgs.pop();
        frameProcess(img);
        procPlantImgs.push_back(img);
    }
}

void PRRWin::phm()
{
    short misMatchCnt = 0;
    while (misMatchCnt != maxDismatchTimes)
    {
        imgCapture->capture();
        qStdOut << "|> misMatchCnt = " << misMatchCnt << Qt::endl;
        otsu(cvFrame);
        displayImgView();
        lightnessPercent = getLightness(cvFrame);
        qStdOut << "|> lp:" << lightnessPercent << Qt::endl;
        qDebug() << "|> lp:" << lightnessPercent;
        if (lightnessPercent < lightnessThres)
        {
            misMatchCnt++;
            qStdOut << "|> misMatchCnt = " << misMatchCnt << Qt::endl;
            qtDelay(1);
            continue;
        }
        else
        {
            plantImgs.push(cvFrame);
            // move up step motor
            qStdOut << "|> lightRegionMeanMaxHeight = " << lightRegionMeanMaxHeight(cvFrame) << Qt::endl;
            while (lightRegionMeanMaxHeight(cvFrame) > 1000)
            {
                qtDelay(3);
                qDebug() << "|> lightRegionMeanMaxHeight = " << lightRegionMeanMaxHeight(cvFrame);
                qStdOut << "|> lightRegionMeanMaxHeight = " << lightRegionMeanMaxHeight(cvFrame) << Qt::endl;
                serialSend(serial0, "up", 400);
                cameraHeight += mapCycleToHeight(400);
                imgCapture->capture();
            }
            phmFinished = false;

            // std::thread control([&]()
            //                     { phmControl(); });
            // std::thread computation([&]()
            //                         { phmComputation(); });
            // control.join();
            // computation.join();

            // while (lightnessPercent >= lightnessThres)
            // {
            //     imgCapture->capture();
            //     displayImgView();
            //     frameProcess(cvFrame);
            //     lightnessPercent = getLightness(cvFrame) / 255;
            //     serialSend(serial0, "distance", 200);
            // }
            // serialSend(serial0, "down", 400);
            while (cameraHeight > 0)
            {
                serialSend(serial0, "down", 400);
                cameraHeight -= mapCycleToHeight(400);
            }
            return;
        }
    }
    qStdOut << "|> Plant Height Measure failed because of initial state" << Qt::endl;
}
#endif

void PRRWin::cameraInit()
{
    // init camera
    if (!isCameraActive)
    {
        qStdOut << "|> camera is not active." << Qt::endl;
        return;
    }
    qStdOut << "|> check camera's activity success." << Qt::endl;
    // init capture
    imgCapture->capture();
    qtDelay(2);
    qStdOut << "|> capture image success." << Qt::endl;
    imgView->setPixmap(QPixmap::fromImage(qtFrame.scaled(imgView->width(), imgView->height(), Qt::KeepAspectRatio)));
    imgView->setScaledContents(true);
    displayImgView();
    // clear plantHeghts
    plantHeights.clear();
}

void PRRWin::setCamera(const QCameraDevice &cameraDevice)
{
    camera.reset(new QCamera(cameraDevice));

    connect(camera.data(), &QCamera::activeChanged, this, &PRRWin::updateCameraActive);
    connect(camera.data(), &QCamera::errorOccurred, this, &PRRWin::displayCameraError);
    captureSession.setCamera(camera.data());
    //    if (!m_mediaRecorder) {
    //        m_mediaRecorder.reset(new QMediaRecorder);
    //        m_captureSession.setRecorder(m_mediaRecorder.data());

    //        connect(m_mediaRecorder.data(), &QMediaRecorder::recorderStateChanged, this,
    //                &Camera::updateRecorderState);
    //        connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this,
    //                &Camera::updateRecordTime);
    //        connect(m_mediaRecorder.data(), &QMediaRecorder::errorChanged, this,
    //                &Camera::displayRecorderError);
    //    }

    if (!imgCapture)
    {
        imgCapture.reset(new QImageCapture);
        captureSession.setImageCapture(imgCapture.get());
        connect(imgCapture.get(), &QImageCapture::imageCaptured, this, &PRRWin::imgProcess);
        // connect(imgCapture.get(), &QImageCapture::errorOccurred, this, &PRRWin::displayCaptureError);
    }

    updateCameraActive(camera->isActive());
    //    updateRecorderState(m_mediaRecorder->recorderState());
    //    readyForCapture(m_imageCapture->isReadyForCapture());

    //    updateCaptureMode();

    if (camera->cameraFormat().isNull())
    {
        auto formats = cameraDevice.videoFormats();
        if (!formats.isEmpty())
        {
            // Choose a decent camera format: Maximum resolution at at least 30 FPS
            // we use 29 FPS to compare against as some cameras report 29.97 FPS...
            QCameraFormat bestFormat;
            for (const auto &fmt : formats)
            {
                if (bestFormat.maxFrameRate() < 29 && fmt.maxFrameRate() > bestFormat.maxFrameRate())
                    bestFormat = fmt;
                else if (bestFormat.maxFrameRate() == fmt.maxFrameRate() &&
                         bestFormat.resolution().width() * bestFormat.resolution().height() <
                             fmt.resolution().width() * fmt.resolution().height())
                    bestFormat = fmt;
            }
            camera->setCameraFormat(bestFormat);
            //            m_mediaRecorder->setVideoFrameRate(bestFormat.maxFrameRate());
        }
    }
    camera->start();
}

void PRRWin::frameProcess(cv::Mat &frame)
{
    otsu(frame);
    findCenters(frame, centers);
    // thread t0([&]()
    //           { kmeansWork(findCenters, centers); });
    // thread t1([&]()
    //           { contoursWork(findPlantContours, hierarchy, filtratedContours); });
    // t0.join();
    // t1.join();
    // cout << "||>> kmeans time cost:" << kmeansWork.microsTimeCost() << endl;
    // cout << "||>> findContours time cost:" << contoursWork.microsTimeCost() << endl;
    // c++ std algor to calculate the mean of centers
    // double meanX = accumulate(centers.begin(), centers.end(), Point2f(0, 0)) / centers.size();

    for (const auto &center : centers)
    {
        cout << "||>> center:" << center.x << "," << center.y << endl;
        // circle(raw, Point(center.x, center.y), 15, Scalar(255,0,0), -1);
        auto tup = getPlantHeight(center, frame, 50, 400);
        plantHeights.push_back(get<1>(tup) - get<0>(tup));
        // line(raw, Point(center.x, get<0>(tup)), Point(center.x, get<1>(tup)), Scalar(0, 255, 0), 3, LINE_8);
    }
}

double PRRWin::mapCycleToHeight(double cycle)
{
    return cycle / 400 * 0.8;
}

/* Slot functions */

// Button Click
void PRRWin::onMeasureClicked()
{
#if _ORANGE_PI_
    serialInit();
#endif
    qStdOut << height << Qt::endl;
    cameraInit();
    imgCapture->capture();
    qtDelay(2);
    qDebug() << cvFrame.empty() << Qt::endl;
    imgView->setPixmap(QPixmap::fromImage(qtFrame.scaled(imgView->width(), imgView->height(), Qt::KeepAspectRatio)));
    imgView->setScaledContents(true);
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

void PRRWin::displayImgView()
{
    ui->stackedWidget->setCurrentIndex(1);
}

// Camera
void PRRWin::startCamera()
{
    camera->start();
}

void PRRWin::closeCamera()
{
    camera->stop();
}

void PRRWin::imgProcess(int id, const QImage &image)
{
    qtFrame = image;
    cvFrame = cv::Mat(image.height(), image.width(), CV_8UC3, (uchar *)image.bits(), image.bytesPerLine());
    // check whether cvFrame is empty
    qDebug() << "||>> cvFrame:" << cvFrame.empty();
}

void PRRWin::updateCameraActive(bool active)
{
    if (active)
    {
        ui->cameraOpen->setEnabled(false);
        ui->cameraClose->setEnabled(true);
        ui->cameraSetting->setEnabled(true);
        isCameraActive = true;
    }
    else
    {
        ui->cameraOpen->setEnabled(true);
        ui->cameraClose->setEnabled(false);
        ui->cameraSetting->setEnabled(false);
        isCameraActive = false;
    }
}

void PRRWin::updateCamera(QAction *action)
{
    setCamera(qvariant_cast<QCameraDevice>(action->data()));
}

void PRRWin::updateCameraList()
{
    ui->cameraList->clear();
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras)
    {
        QAction *videoDeviceAction = new QAction(cameraDevice.description(), camerasGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
        {
            videoDeviceAction->setChecked(true);
        }
        ui->cameraList->addAction(videoDeviceAction);
    }
}

void PRRWin::displayCameraError()
{
    if (camera->error() != QCamera::NoError)
        QMessageBox::warning(this, tr("Camera Error"), camera->errorString());
}

void PRRWin::displayCaptureError(int id, const QImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
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
