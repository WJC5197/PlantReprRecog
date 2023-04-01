#include "terminal.h"

Terminal::Terminal(QTextEdit *textEdit)
    : QIODevice(textEdit),textEdit(textEdit)
{

}

qint64 Terminal::readData(char* data, qint64 maxSize)
{
    return 0;
}

qint64 Terminal::writeData(const char* data, qint64 maxSize)
{
    textEdit->moveCursor(QTextCursor::End);
    textEdit->insertPlainText(QString::fromUtf8(data, maxSize));
    return maxSize;
}
