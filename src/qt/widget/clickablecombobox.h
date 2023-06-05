#ifndef CLICKABLECOMBOBOX_H
#define CLICKABLECOMBOBOX_H

#include <QObject>
#include <QComboBox>
#include <QMouseEvent>

class ClickableComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit ClickableComboBox(QWidget *parent = nullptr);
protected:
    virtual void mousePressEvent(QMouseEvent *e);
signals:
    void clicked();
};

#endif // CLICKABLECOMBOBOX_H
