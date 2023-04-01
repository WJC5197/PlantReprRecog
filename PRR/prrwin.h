#ifndef PRRWIN_H
#define PRRWIN_H

#include <QMainWindow>
#include "requires.h"
#include "terminal.h"

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
    // Interact
    Ui::PRRWin* ui;
    QPushButton* detectBtn;
    QPushButton* videoBtn;
    QPushButton* calibrateBtn;
    QPushButton* cleanBtn;
    QLineEdit* widthSet;
    QLineEdit* heightSet;
    QLineEdit* fpsSet;
    Terminal* terminal;
    QTextStream qStdOut;
    // Img View
    QLabel* imgView;
    QVideoWidget* videoView;
    QTimer* timer;

    // Camera
    QMediaDevices devices;
    QMediaCaptureSession captureSession;
    QScopedPointer<QCamera> camera;
    QMenu* cameraList;
    QStackedWidget* stackedView;
//    QScopedPointer<QImageCapture> imageCapture;

    int width = 640;
    int height = 480;
    int fps = 10;

private slots:
    // btn
    void onDetectClicked();
    void onVideoClicked();
    void onCalibrateClicked();
    void onCleanClicked();
    // input
    void onWidthSetEditingFinished();
    void onHeightSetEditingFinished();
    void onfpsSetEditingFinished();
//    // view
    void displayVideoView();
    void displayImageView();
//    // camera
    void setCamera(const QCameraDevice&);
    void displayCameraError();
    void updateCameraList();
    void startCamera();
    void stopCamera();
    void updateCameraActive(bool);
    void updateCamera();
};
#endif // PRRWIN_H

