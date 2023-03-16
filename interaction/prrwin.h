#ifndef PRRWIN_H
#define PRRWIN_H

#include <QMainWindow>
#include "requires.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PRRWin; }
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
    void updateCameraDevice(QAction *action);
    void updateCameras();
    void setCamera(const QCameraDevice &cameraDevice);
private:
    // Interact
    Ui::PRRWin* ui;
    QPushButton* detectBtn;
    QPushButton* videoBtn;
    QPushButton* calibrateBtn;
    QLineEdit* widthSet;
    QLineEdit* heightSet;
    QLineEdit* fpsSet;
    QTextEdit* terminal;
    // Img View
    QLabel* imgView;
    QVideoWidget* videoView;
    QTimer* timer;
    // Camera
    QActionGroup* cameraList;
    QScopedPointer<QCamera> m_camera;
    int width = 640;
    int height = 480;
    int fps = 10;


private slots:
    // btn
    void onDetectClicked();
    void onVideoClicked();
    // input
    void onWidthSetEditingFinished();
    void onHeightSetEditingFinished();
    void onfpsSetEditingFinished();
};
#endif // PRRWIN_H
