#ifndef PRRWIN_H
#define PRRWIN_H

#include <QMainWindow>
#include "requires.h"
#include "terminal.h"
#include "imagesettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PRRWin; }
class QActionGroup;
QT_END_NAMESPACE

class PRRWin : public QMainWindow
{
    Q_OBJECT
public:
    PRRWin(QWidget *parent = nullptr);
    ~PRRWin();
    // Img View
    void displayImg();
    void displayVideo();
    // QT Camera
    void cameraInit();
private:
    Ui::PRRWin* ui;
    // state
    bool isCapturingImg = false;
    // Btn
    QPushButton* detectBtn;
    QPushButton* videoBtn;
    QPushButton* calibrateBtn;
    QPushButton* cleanBtn;
    // Camera
    QActionGroup* camerasGroup;
    QAction* cameraClose;
    QAction* cameraOpen;
    QAction* cameraSetting;

    // terminal
    Terminal* terminal;
    QTextStream qStdOut;
    
    // Img View
    QLabel* imgView;
    QVideoWidget* videoView;
    QTimer* timer;

    // hardware
        // Serial
    int serial0;
    
    // process
    double paceDis = 1000; // every pace's displacement
    double cameraHeight = 1000; // store camera position
    short picNum = 0;
    std::vector<double> plantHeights;
    std::vector<cv::Mat> plantImgs; // store for height processing
    double lightnessPercent = 0;
    bool fstEntry = false; // if enter Plant Height Measure Process first time, then set true
    short maxDismatchTimes = 5; // max times of dismatch, if exceed, then break
    // flow control
    bool phmFinished = false;

    // camera
    QMediaDevices devices;
    QMediaCaptureSession captureSession;
    QScopedPointer<QImageCapture> imgCapture;
    QScopedPointer<QCamera> camera;
    QMenu* cameraList;
    QStackedWidget* stackedView;
    QImage qtFrame;
    cv::Mat cvFrame;
    int width = 640;
    int height = 480;
    int fps = 10;
    double lightnessThres = 0.1;
    bool isCameraActive = false;
    bool isFstCapture = false;

    // function
    void displayVideoView();
    void displayImgView();

    #if _ORANGE_PI_
    void serialInit();
    void phmControl();
    #endif
    
    void phmInit();
    void phmComputation();
    void plantHeightMeasure();
    void frameProcess(cv::Mat&);

private slots:
    // btn
    void onMeasureClicked();
    void onVideoClicked();
    void onCalibrateClicked();
    void onCleanClicked();
    // camera
    void setCamera(const QCameraDevice&);
    void imgProcess(int, const QImage&);
    void startCamera();
    void closeCamera();
    void updateCameraList();
    void updateCameraActive(bool);
    void updateCamera(QAction*);
    void cfgImgSettings();
    // error
    void displayCameraError();
    void displayCaptureError(int, const QImageCapture::Error, const QString&);
};
#endif // PRRWIN_H
