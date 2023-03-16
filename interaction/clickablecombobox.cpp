#include "clickablecombobox.h"
ClickableComboBox::ClickableComboBox(QWidget *parent):QComboBox(parent)
{

}


void ClickableComboBox::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit clicked();
    }

    QComboBox::mousePressEvent(event);
}
