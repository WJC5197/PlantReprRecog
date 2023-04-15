#ifndef _QTUTILITY_HPP_
#define _QTUTILITY_HPP_

#include "requires.h"

bool isIntInRange(QString s, int low, int high)
{
    bool ok;
    int num = s.toInt(&ok);
    if (!ok)
    {
        // Failed to convert string to integer
        return false;
    }
    return num >= low && num <= high;
}

void delay(double n)
{
    QTime dieTime= QTime::currentTime().addSecs(n);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

//std::vector<std::string> getCameraList()
//{
//    std::vector<std::string> cameraNames;

//    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

//    foreach (const QCameraInfo &cameraInfo, cameras) {
//        qDebug() << cameras.indexOf(QCameraInfo::defaultCamera());
//        QString name = cameraInfo.description();
//        qDebug() << name;
//        cameraNames.push_back(name.toStdString());
//    }

//    return cameraNames;
//}

#endif
