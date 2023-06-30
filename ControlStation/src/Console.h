#ifndef CONSOLE_H
#define CONSOLE_H

#include <QTextEdit>

class Console : public QTextEdit
{
    Q_OBJECT

public:
    Console(QWidget *parent = nullptr);

private:
    void writeOutput(QString output, QColor color);
};

#endif // CONSOLE_H
