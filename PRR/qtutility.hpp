#ifndef _QTUTILITY_HPP_
#define _QTUTILITY_HPP_

#include "requires.h"

void qtDelay(double n)
{
    QTime dieTime= QTime::currentTime().addSecs(n);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

#endif
