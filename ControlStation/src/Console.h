#ifndef CONSOLE_H
#define CONSOLE_H

#include "OutputWatcher.h"

#include <QTextEdit>

class Console : public QTextEdit
{
    Q_OBJECT

public:
    Console(const OutputWatcher *stdoutWatcher, const OutputWatcher *stderrWatcher, QWidget *parent = nullptr);

private:
    void writeOutput(QString output, QColor color);
};

#endif // CONSOLE_H
