#include "linenumberarea.h"

#include <ctextedit.h>

QSize LineNumberArea::sizeHint() const
{
    return QSize(textEdit->lineNumberAreaWidth(), 0);
}

LineNumberArea::LineNumberArea(CTextEdit *editor) : textEdit(editor){}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    textEdit->lineNumberAreaPaintEvent(event);
}
