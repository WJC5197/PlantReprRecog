#ifndef PRRWIN_H
#define PRRWIN_H

#include <QMainWindow>
#include <QMqttClient>
#include "../requires.h"
#include "../widget/terminal.h"
#include "../widget/clickablecombobox.h"

#define _ORANGE_PI_ 0

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PRRWin;
}
class QActionGroup;
QT_END_NAMESPACE

class PRRWin : public QMainWindow
{
    Q_OBJECT
public:
    PRRWin(QWidget *parent = nullptr);
    ~PRRWin();

private:
    //// UI
    Ui::PRRWin *ui;
    // state
    bool isCapturingImg = false;
    // Btn
    QPushButton *continuousBtn;
    QPushButton *singleBtn;
    QPushButton *videoBtn;
    QPushButton *calibrateBtn;
    QPushButton *cleanBtn;

    // Camera
    QActionGroup *camerasGroup;
    QAction *cameraClose;
    QAction *cameraOpen;
    QAction *cameraSetting;

    // terminal
    Terminal *terminal;
    QTextStream qStdOut;

    // Img View
    QLabel *imgView;
    QVideoWidget *videoView;
    QTimer *timer;

    // camera
    QMenu *cameraList;
    QStackedWidget *stackedView;

    //// Hardware
    // serial
    int serial0;
    // mqtt
    QMqttClient *mqttClient;

    // camera
    QMediaDevices devices;
    QMediaCaptureSession captureSession;
    QScopedPointer<QImageCapture> imgCapture;
    QScopedPointer<QCamera> camera;
    bool isCameraActive = false;

    //// Process
    // phm
    double paceDis = 200;    // every pace's displacement
    double initHeight = 0;   // initial height, unit:cm
    double cameraHeight = 0; // store camera position, unit:cm
    std::vector<std::tuple<int, int>> plantHeights;
    std::queue<cv::Mat> plantImgs; // store for height processing
    std::vector<cv::Mat> procPlantImgs;
    double lightnessPercent = 0;
    short maxDismatchTimes = 5; // max times of dismatch, if exceed, then break

    // thread
    bool phmFinished = false;
    std::mutex mutex;
    std::condition_variable cond;

    // img
    QImage qtFrame;
    cv::Mat cvFrame;
    int width;
    int height;
    int fps = 10;
    double lightnessThres = 0;

    //// Function
    // ui
    void displayImg();
    void displayVideo();
    void displayVideoView();
    void displayImgView();
    void displayImgView(const cv::Mat &);
    // camera
    void initCamera();
    // process
    void frameProcess(cv::Mat &);
    double mapCycleToHeight(double);
    void phmPlantRegion();

    // prrwin
    void serialSend(int, const char[], int);
    void serialInit();
    void calcRealHeight();
    void phmControl();
    void phmComputation();
    void phm();

private slots:
    // btn
    void onSingleClicked();
    void onContinuousClicked();
    void onVideoClicked();
    void onCalibrateClicked();
    void onCleanClicked();
    // camera
    void setCamera(const QCameraDevice &);
    void imgProcess(int, const QImage &);
    void startCamera();
    void closeCamera();
    void updateCameraList();
    void updateCameraActive(bool);
    void updateCamera(QAction *);
    void cfgImgSettings();
    // error
    void displayCameraError();
    void displayCaptureError(int, const QImageCapture::Error, const QString &);
};
#endif // PRRWIN_H