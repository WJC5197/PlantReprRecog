#include "prrwin.h"
#include "./ui_prrwin.h"

#include "../qtutility.hpp"

// Camera
void PRRWin::initCamera()
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

void PRRWin::startCamera()
{
    camera->start();
}

void PRRWin::closeCamera()
{
    camera->stop();
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
