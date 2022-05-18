#include "ctextedit.h"

#include <QFocusEvent>
#include <QAbstractItemView>
#include <QScrollBar>

#include <QTextBlock>

void CTextEdit::keyPressEvent(QKeyEvent* event)
{

    char key = event->key();
    /*
    if(event->key() == '`')
    {
      DisplayMessage(QcharToString(lastKeyPressed));
      return;
    }
    if(key == '1')
    {
        int pos = this->textCursor().position();
        int targetKey = this->toPlainText()[pos].unicode();        
        DisplayMessage(QcharToString(targetKey));
        return;
    }*/


    switch(event->key())
    {
    case '(':{
        this->insertPlainText("()");
        auto cursor = this->textCursor();
        cursor.movePosition(MOVE::Left);
        this->setTextCursor(cursor);
        lastKeyPressed = '(';
        return;
    }
    case ')':{
     if(lastKeyPressed != '('){
         this->insertPlainText(")");
     }
     else{
         auto cursor = this->textCursor();
         cursor.movePosition(MOVE::Right);
         this->setTextCursor(cursor);
     }
     lastKeyPressed = ')';
        return;
    }
    case '{':{
        this->insertPlainText("{}");
        auto cursor = this->textCursor();
        cursor.movePosition(MOVE::Left);
        this->setTextCursor(cursor);
     lastKeyPressed = '{';
     return;
    }
    case '}':{
        if(lastKeyPressed != '{'){
            this->insertPlainText("}");
        }
        else{
            auto cursor = this->textCursor();
            cursor.movePosition(MOVE::Right);
            this->setTextCursor(cursor);
        }
        lastKeyPressed = '}';
        return;
    }
    default:
    break;
    }


    //completer code:----

    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (event->key()) {
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            event->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    const bool isShortcut = (event->modifiers().testFlag(Qt::ControlModifier) && event->key() == Qt::Key_E); // CTRL+E
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(event);

    const bool ctrlOrShift = event->modifiers().testFlag(Qt::ControlModifier) ||
                             event->modifiers().testFlag(Qt::ShiftModifier);
    if (!c || (ctrlOrShift && event->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    const bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || event->text().isEmpty()|| completionPrefix.length() < 1
                      || eow.contains(event->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);

    //completer code...

    lastKeyPressed = event->key();



    //QTextEdit::keyPressEvent(event);
}

void CTextEdit::handleEnterKey()
{
    int indentAmount = currentLineIndentAmount();
    QString indent = QString(indentAmount, ' ');


    if(lastKeyPressed == '{')
    {
        this->setTextCursor(moveCursor(MOVE::Left));
        this->insertAllPlainText("\n", indent);
        this->setTextCursor(moveCursor(MOVE::Right));
        this->insertAllPlainText("\n\n", indent);
        this->setTextCursor(moveCursor(MOVE::Up));
        this->insertPlainText(indent);
        this->insertPlainText(tab);
        return;
    }

    if(lastKeyPressed == '}')
    {
        auto cursor = this->textCursor();
        cursor.movePosition(MOVE::Left, MOVEMODE::KeepAnchor, 2);
        if(cursor.selectedText() == "{}")
        {
            cursor.removeSelectedText();
            this->insertPlainText("\n" + indent + "{" + indent + "\n" + indent + "}");
            cursor = this->textCursor();
            cursor.movePosition(MOVE::Left);
            cursor.movePosition(MOVE::Up);
            this->setTextCursor(cursor);
            this->insertPlainText(tab);
        }
        else
        {
            this->insertAllPlainText("\n",indent);
        }
        return;
    }


    auto cursor = this->textCursor();
    cursor.movePosition(MOVE::Left, MOVEMODE::KeepAnchor);

    switch(cursor.selectedText()[0].unicode())
    {
    case '{':
        this->insertAllPlainText("\n\n", indent);
        moveCursor(MOVE::Up);
        this->insertAllPlainText(tab);
    return;
     case ':':
        this->insertAllPlainText("\n",indent);
        if(isPythonFile)
            this->insertPlainText(tab);
     return;
    case '}':
        cursor = this->textCursor();
        cursor.movePosition(MOVE::Left, MOVEMODE::KeepAnchor, 2);
        if(cursor.selectedText() == "{}")
        {
            cursor.removeSelectedText();
            this->insertAllPlainText("\n",indent,"{\n\n", indent, "}");
            cursor = this->textCursor();
            cursor.movePosition(MOVE::Left);
            cursor.movePosition(MOVE::Up);
            this->setTextCursor(cursor);
            this->insertPlainText(tab);
        }
        else
        {
            this->insertAllPlainText("\n",indent);
        }
    return;
    default:
        this->insertAllPlainText("\n", indent);
    return;
    }


}

QTextCursor CTextEdit::moveCursor(QTextCursor::MoveOperation moveOp, QTextCursor::MoveMode moveMode)
{
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(moveOp, moveMode);
    return cursor;
}

void CTextEdit::DisplayMessage(const QString& message)
{
    static QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

int CTextEdit::currentLineIndentAmount()
{
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(MOVE::StartOfLine);
    cursor.movePosition(MOVE::EndOfLine, MOVEMODE::KeepAnchor);
    QString selectedText = cursor.selectedText();

    int diff = 0;

    for(int i = 0; i < selectedText.length(); i++)
    {        
        if(selectedText[i] == ' ')
            diff++;
        else if(selectedText == '\t')
            diff += tab.length();
        else{
           // DisplayMessage(QStringLiteral("Selected text was %1").arg(QVariant(selectedText[i]).toInt()));
            break;
        }

    }
    //if(diff == selectedText.length())
      //  diff = 0;
    return diff;
}

int CTextEdit::distanceFromStartOfLine()
{
    QTextCursor cursor = this->textCursor();
    int posA = cursor.position();
    cursor.movePosition(QTextCursor::MoveOperation::StartOfLine);
    int posB = cursor.position();
    return posA - posB;
}


void CTextEdit::SetUpShortcuts()
{
    QShortcut* duplicateLine = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
    QObject::connect(duplicateLine, &QShortcut::activated,
    [this]
    {
        if(!this->hasFocus())
           return;
        QTextCursor cursor = this->textCursor();
        cursor.movePosition(MOVE::StartOfLine, MOVEMODE::KeepAnchor);
        QString target = cursor.selectedText();
        this->insertPlainText("\n" + target);
    });

}

void CTextEdit::duplicateLine()
{
    using MOVE = QTextCursor::MoveOperation;
    using MOVEMODE = QTextCursor::MoveMode;

    auto cursor = this->textCursor();
    cursor.movePosition(MOVE::EndOfLine);
    this->setTextCursor(cursor);
    cursor.movePosition(MOVE::StartOfLine, MOVEMODE::KeepAnchor);
    QString targetText = cursor.selectedText();
    this->insertPlainText("\n");
    this->insertPlainText(targetText);
}


QString QcharToString(QChar c)
{
    return QVariant(c.unicode()).toString();
}

QCompleter* CTextEdit::completer() const
{
    return c;
}


void CTextEdit::setCompleter(QCompleter *completer)
{
    if (c)
        c->disconnect(this);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, QOverload<const QString &>::of(&QCompleter::activated),
                     this, &CTextEdit::insertCompletion);
}

void CTextEdit::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CTextEdit::focusInEvent(QFocusEvent* e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CTextEdit::SetUpLineNumberAreaCode()
{
    lineNumberArea = new LineNumberArea(this);
    connect(this, &CTextEdit::blockCountChanged, this, &CTextEdit::updateLineNumberAreaWidth);
    connect(this, &CTextEdit::updateRequest, this, &CTextEdit::updateLineNumberArea);
    connect(this, &CTextEdit::cursorPositionChanged, this, &CTextEdit::highlightCurrentLine);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CTextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CTextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CTextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CTextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(190); //160

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
     int blockNumber = block.blockNumber();
     int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
     int bottom = top + qRound(blockBoundingRect(block).height());

     while (block.isValid() && top <= event->rect().bottom()) {
         if (block.isVisible() && bottom >= event->rect().top()) {

             QString number = QString::number(blockNumber + 1);
             painter.setPen(Qt::black);
             painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                              Qt::AlignRight, number);
         }

         block = block.next();
         top = bottom;
         bottom = top + qRound(blockBoundingRect(block).height());
         ++blockNumber;
     }
}
