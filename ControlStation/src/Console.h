#ifndef CONSOLE_H
#define CONSOLE_H

#include "OutputWatcher.h"

#include <QTextEdit>

class Console : public QTextEdit
{
    Q_OBJECT

public:
    Console(OutputWatcher *stdoutWatcher, OutputWatcher *stderrWatcher, QWidget *parent = nullptr);

private:
    void writeOutput(char *output, int n, QColor color);
};

#endif // CONSOLE_H
