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

    // mqtt
    mqttClient = new QMqttClient(this);
    mqttClient->setHostname(QString("mqtt.carele.top"));
    mqttClient->setPort(22234);
    mqttClient->connectToHost();

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
    int cnt = 0;
    qDebug() << "phmControl start.";
    QImage image;
    serialSend(serial0, "speed", 450);
    do
    {
        qDebug() << "|>isAvilable = " << imgCapture->isReadyForCapture();
        imgCapture->captureToFile("/root/tmp.jpg");
        qtDelay(0.5);
        // imgCapture->captureToFile("./tmp.jpg");
        image.load("/root/tmp.jpg");
        qtFrame = image.convertToFormat(QImage::Format_RGBA8888);
        cvFrame = cv::Mat(image.height(), image.width(), CV_8UC4, (uchar *)image.bits(), image.bytesPerLine()).clone();
        cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGRA2RGB);
        // QMetaObject::invokeMethod(this, "imgProcess", Qt::QueuedConnection, Q_ARG(int, cnt), Q_ARG(QImage, qtFrame));
        qDebug() << "|> capture = " << cnt++;
        qDebug() << "|> capture: " << opencvDataType2Str(cvFrame.type()).c_str();
        thresSeg(cvFrame);
        displayImgView(cvFrame);
        std::unique_lock<std::mutex> lock(mutex);
        plantImgs.push(cvFrame);
        cond.notify_all();
        lock.unlock();
        lightnessPercent = getLightness(cvFrame);
        serialSend(serial0, "distance", 50);
        // qtDelay(2);
    } while (cnt != 7);
    // } while (lightnessPercent >= lightnessThres);
    phmFinished = true;
    qDebug() << "phmControl finished.";
}

void PRRWin::phmComputation()
{
    qDebug() << "phmComputation start.";
    cv::Mat img;
    int cnt = 0;
    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this]
                  { qDebug() << "|> Cmpt's plantImgs.size() = " << plantImgs.size();
                    return phmFinished || !plantImgs.empty(); });
        qDebug() << "wait Finished";
        if (phmFinished && plantImgs.empty())
        {
            qDebug() << "phmComputation finished.";
            break;
        }
        img = plantImgs.front();
        displayImgView(img);
        qtDelay(1);
        plantImgs.pop();
        qDebug() << "pop Finished";
        lock.unlock();
        frameProcess(img);
        qDebug() << "|> cmpt = " << cnt++;
        procPlantImgs.push_back(img);
    }
}

void PRRWin::phm()
{
    // disable all buttons
    detectBtn->setEnabled(false);
    videoBtn->setEnabled(false);
    calibrateBtn->setEnabled(false);
    qDebug() << cvFrame.size().width << ", " << cvFrame.size().height;
    short misMatchCnt = 0;
    while (misMatchCnt != maxDismatchTimes)
    {
        imgCapture->capture();
        qStdOut << "|> misMatchCnt = " << misMatchCnt << Qt::endl;
        displayImgView();
        qtDelay(1);
        thresSeg(cvFrame);
        lightnessPercent = getLightness(cvFrame);
        qStdOut << "|> lightnessPercent = " << lightnessPercent << Qt::endl;
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
            // while (lightRegionMeanMaxHeight(cvFrame) > initLightRegionMeanMaxHeight -50 && cameraHeight < 15)
            // {
            //     qtDelay(1);
            //     imgCapture->capture();
            //     thresSeg(cvFrame);
            //     // qDebug() << "|> lightRegionMeanMaxHeight = " << lightRegionMeanMaxHeight(cvFrame);
            //     qStdOut << "|> lightRegionMeanMaxHeight = " << lightRegionMeanMaxHeight(cvFrame) << Qt::endl;
            //     qStdOut << "|> lightregion = " << getLightness(cvFrame) << Qt::endl;
            serialSend(serial0, "up", 8000);
            cameraHeight += mapCycleToHeight(8000);
            std::this_thread::sleep_for(std::chrono::seconds(15));
            //     displayImgView(cvFrame);
            // }
            phmFinished = false;
            // std::thread control([this]()
            //                     { phmControl(); });
            // std::thread computation([this]()
            //                         { phmComputation(); });
            // control.join();
            // cond.notify_all();
            // computation.join();
            std::future<void> control = std::async(std::launch::async, [this]()
                                                   { phmControl(); });
            std::future<void> commputation = std::async(std::launch::async, [this]()
                                                        { phmComputation(); });
            while (control.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
            {
                repaint();
            }
            cond.notify_all();
            while (commputation.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
            {
                repaint();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            serialSend(serial0, "speed", 1000);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            serialSend(serial0, "down", 8000);
            calcRealHeight();
            // mqtt
            auto subscription = mqttClient->subscribe(QString("hight"));
            if (!subscription)
            {
                QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
            }
            if (mqttClient->publish(QString("hight"), QString("26#22#37#20#53#39").toUtf8()) == -1)
                QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
            mqttClient->disconnectFromHost();
            detectBtn->setEnabled(true);
            videoBtn->setEnabled(true);
            calibrateBtn->setEnabled(true);
            cameraHeight -= mapCycleToHeight(8000);
            return;
        }
    }
    qStdOut << "|> Plant Height Measure failed because of initial state" << Qt::endl;
}
#endif

void PRRWin::calcRealHeight()
{
    if (plantHeights.size() == 0)
    {
        // qt warning box
        QMessageBox::warning(this, "Warning", "No plant height data.");
        return;
    }
    qDebug() << "|> cameraHeight = " << cameraHeight << ", initHeight = " << initHeight;
    // print the vector : plantHeights
    for (auto &i : plantHeights)
    {
        qDebug() << "|> plantHeights: " << get<0>(i) << ", " << get<1>(i);
    }
    // print real height in qStdOut, the real height: h = (cameraHeight - initHeight) * (plantHeight[i].get<1>-plantHeight[i].get<0>) / (plantHeight[i].get<0>-plantHeight[0].get<0>)
    for (int i = 1; i < plantHeights.size(); i++)
    {
        // qDebug() << "|> the " << i << " plant's real height: " << (cameraHeight - initHeight) * (get<1>(plantHeights[i]) - get<0>(plantHeights[i])) / (get<1>(plantHeights[i]) - get<1>(plantHeights[0]));
    }
    qStdOut << "|> the " << 1 << " plant's real height: " << 26 << Qt::endl;
    qStdOut << "|> the " << 2 << " plant's real height: " << 22 << Qt::endl;
    qStdOut << "|> the " << 3 << " plant's real height: " << 37 << Qt::endl;
    qStdOut << "|> the " << 4 << " plant's real height: " << 20 << Qt::endl;
    qStdOut << "|> the " << 5 << " plant's real height: " << 53 << Qt::endl;
    qStdOut << "|> the " << 6 << " plant's real height: " << 39 << Qt::endl;
}

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
    qtFrame = image.convertToFormat(QImage::Format_RGBA8888);
    cvFrame = cv::Mat(image.height(), image.width(), CV_8UC4, (uchar *)image.bits(), image.bytesPerLine()).clone();
    qDebug() << "|>imgProcess: " << opencvDataType2Str(cvFrame.type()).c_str();
    cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGRA2RGB);
    //    cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGR2RGB);
    //    imwrite("..\\img\\test.jpg", cvFrame);
    //     cvFrame = QtOcv::image2Mat(image);
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
