#ifndef TERMINAL_H
#define TERMINAL_H

#include "../requires.h"

class Terminal : public QIODevice
{
    Q_OBJECT
public:
    Terminal(QTextEdit* textEdit);
private:
    QTextEdit* textEdit;
    qint64 readData(char*, qint64) override;
    qint64 writeData(const char*, qint64) override;

};

#endif // TERMINAL_H
