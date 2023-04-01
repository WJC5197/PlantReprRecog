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
        cleanBtn = ui->clean;
        // camera
//        imgView = ui->imgView;
//        videoView = ui->videoView;
        cameraList = ui->cameraList;
//        cameraOpe = ui->cameraOpe;
        // input
        widthSet = ui->widthSet;
        heightSet = ui->heightSet;
        fpsSet = ui->fpsSet;
        // terminal ouput
        terminal = new Terminal(ui->terminal);
        terminal->open(QIODevice::WriteOnly);
        qStdOut.setDevice(terminal);

        qDebug() << terminal->isOpen();
        // stack widget
        stackedView = ui->stackedWidget;

    updateCameraList();

    // connect
        // btn
        connect(detectBtn, SIGNAL(clicked()), this, SLOT(onDetectClicked()));
        connect(videoBtn, SIGNAL(clicked()), this, SLOT(onVideoClicked()));
        connect(calibrateBtn, SIGNAL(clicked()), this, SLOT(onCalibrateClicked()));
        connect(cleanBtn, SIGNAL(clicked()), this, SLOT(onCleanClicked()));

        // camera
        connect(&devices, SIGNAL(videoInputsChanged()), this, SLOT(updateCameraList()));

        connect(cameraList, SIGNAL(triggered(QAction*)), this, SLOT(updateCamera()));

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

/* Slot functions */

    // Button Click
    void PRRWin::onDetectClicked()
    {
        cv::Mat img = cv::imread("D:/_Proj/CC/PlantReprRecog/img/1.jpg");
        std::cout << opencvDataType2Str(img.type()) << std::endl;
        cvtColor(img, img, cv::COLOR_BGR2RGB);

    //    ui->imgView->setPixmap(
    //        QPixmap::fromImage(
    //           QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)
    //        )
    //    );
        cv::waitKey();
    }

    void PRRWin::onVideoClicked()
    {
        qStdOut << "test" << Qt::endl;
    }

    void PRRWin::onCalibrateClicked()
    {

    }

    void PRRWin::onCleanClicked()
    {
        ui->terminal->clear();
    }

    // Input
    void PRRWin::onWidthSetEditingFinished()
    {
        QString widthStr = widthSet->text();
        if(!isIntInRange(widthStr, 640, 2592))
        {
            return;
        }
    //    camera.set(cv::CAP_PROP_FRAME_WIDTH, widthStr.toInt());
    }


    void PRRWin::onHeightSetEditingFinished()
    {
        QString heightStr = heightSet->text();
        if(!isIntInRange(heightStr, 480, 1944))
        {
            return;
        }
    //    camera.set(cv::CAP_PROP_FRAME_HEIGHT, heightStr.toInt());
    }


    void PRRWin::onfpsSetEditingFinished()
    {
        QString fpsStr = fpsSet->text();
        if(!isIntInRange(fpsStr, 5, 30))
        {
            return;
        }
    //    camera.set(cv::CAP_PROP_FPS, fpsStr.toInt());
    }


void PRRWin::startCamera()
{
    camera->start();
}

void PRRWin::stopCamera()
{
    camera->stop();
}

void PRRWin::updateCameraActive(bool active)
{
    if (active) {
        ui->openCamera->setEnabled(false);
        ui->closeCamera->setEnabled(true);
//        ui->captureWidget->setEnabled(true); adjust it to your kind
        ui->cameraSetting->setEnabled(true);
    } else {
        ui->openCamera->setEnabled(true);
        ui->closeCamera->setEnabled(false);
//        ui->captureWidget->setEnabled(false);
        ui->cameraSetting->setEnabled(false);
    }
}

void PRRWin::setCamera(const QCameraDevice &cameraDevice)
{
    camera.reset(new QCamera(cameraDevice));
    captureSession.setCamera(camera.data());

//    connect(camera.data(), &QCamera::activeChanged, this, &PRRWin::updateCameraActive);
//    connect(camera.data(), &QCamera::errorOccurred, this, &PRRWin::displayCameraError);

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

//    if (!m_imageCapture) {
//        m_imageCapture.reset(new QImageCapture);
//        m_captureSession.setImageCapture(m_imageCapture.get());
//        connect(m_imageCapture.get(), &QImageCapture::readyForCaptureChanged, this,
//                &Camera::readyForCapture);
//        connect(m_imageCapture.get(), &QImageCapture::imageCaptured, this,
//                &Camera::processCapturedImage);
//        connect(m_imageCapture.get(), &QImageCapture::imageSaved, this, &Camera::imageSaved);
//        connect(m_imageCapture.get(), &QImageCapture::errorOccurred, this,
//                &Camera::displayCaptureError);
//    }

//    captureSession.setVideoOutput(ui->videoView);

    updateCameraActive(camera->isActive());
//    updateRecorderState(m_mediaRecorder->recorderState());
//    readyForCapture(m_imageCapture->isReadyForCapture());

//    updateCaptureMode();

    if (camera->cameraFormat().isNull()) {
        auto formats = cameraDevice.videoFormats();
        if (!formats.isEmpty()) {
            // Choose a decent camera format: Maximum resolution at at least 30 FPS
            // we use 29 FPS to compare against as some cameras report 29.97 FPS...
            QCameraFormat bestFormat;
            for (const auto &fmt : formats) {
                if (bestFormat.maxFrameRate() < 29 && fmt.maxFrameRate() > bestFormat.maxFrameRate())
                    bestFormat = fmt;
                else if (bestFormat.maxFrameRate() == fmt.maxFrameRate() &&
                         bestFormat.resolution().width()*bestFormat.resolution().height() <
                             fmt.resolution().width()*fmt.resolution().height())
                    bestFormat = fmt;
            }

            camera->setCameraFormat(bestFormat);
//            m_mediaRecorder->setVideoFrameRate(bestFormat.maxFrameRate());
        }
    }
    camera->start();
}

void PRRWin::updateCamera()
{
    QAction* action = qobject_cast<QAction*>(sender());
    setCamera(qvariant_cast<QCameraDevice>(action->data()));
}

void PRRWin::displayCameraError()
{
    if (camera->error() != QCamera::NoError)
        QMessageBox::warning(this, tr("Camera Error"), camera->errorString());
}

void PRRWin::updateCameraList()
{
    ui->cameraList->clear();
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras) {
        QAction* videoDeviceAction = new QAction(cameraDevice.description(), cameraList);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
            videoDeviceAction->setChecked(true);

        ui->cameraList->addAction(videoDeviceAction);
    }
}

void PRRWin::displayVideoView()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void PRRWin::displayImageView()
{
    ui->stackedWidget->setCurrentIndex(1);
}


