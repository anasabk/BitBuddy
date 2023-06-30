#include "Console.h"
#include "constants.h"
#include "OutputWatcher.h"

#include <QScrollBar>

Console::Console(QWidget *parent) :
    QTextEdit(parent)
{
    connect(&OutputWatcher::stdoutWatcher, &OutputWatcher::outputRead, this, [this](QString output) {
        writeOutput(output, QColor::fromString(constants::white));
    });
    connect(&OutputWatcher::stderrWatcher, &OutputWatcher::outputRead, this, [this](QString output) {
        writeOutput(output, QColor::fromString(constants::red));
    });

    setStyleSheet("border: 1px solid #202020");
    setFixedWidth(500);
    setReadOnly(true);

    QTextBlockFormat blockFormat = textCursor().blockFormat();
    blockFormat.setLineHeight(120, QTextBlockFormat::ProportionalHeight);
    textCursor().setBlockFormat(blockFormat);
}

void Console::writeOutput(QString output, QColor color)
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
}
