#include "Console.h"

#include <QScrollBar>

Console::Console(OutputWatcher *stdoutWatcher, OutputWatcher *stderrWatcher, QWidget *parent) :
    QTextEdit(parent)
{
    connect(stdoutWatcher, &OutputWatcher::outputRead, this, [this](char *output, int n) {
        writeOutput(output, n, QColor::fromString("#e0e0e0"));
    });
    connect(stderrWatcher, &OutputWatcher::outputRead, this, [this](char *output, int n) {
        writeOutput(output, n, QColor::fromString("#ff3030"));
    });

    setStyleSheet("border: 1px solid #202020");
    setFixedWidth(500);
    setReadOnly(true);

    QTextBlockFormat blockFormat = textCursor().blockFormat();
    blockFormat.setLineHeight(120, QTextBlockFormat::ProportionalHeight);
    textCursor().setBlockFormat(blockFormat);
}

void Console::writeOutput(char *output, int n, QColor color)
{
    QScrollBar *scrollBar = verticalScrollBar();
    int sliderPosition = scrollBar->sliderPosition();
    bool atEnd = sliderPosition > scrollBar->maximum() - 10;

    moveCursor(QTextCursor::End);
    setTextColor(color);
    insertPlainText(output);
    moveCursor(QTextCursor::End);

    if (!atEnd)
        scrollBar->setSliderPosition(sliderPosition);

    free(output);
}
