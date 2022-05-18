#ifndef CTEXTEDIT_H
#define CTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTextCursor>
#include <QShortcut>
#include <QVector>
#include <QCompleter>
#include <QPlainTextEdit>
#include <linenumberarea.h>

using MOVE = QTextCursor::MoveOperation;
using MOVEMODE = QTextCursor::MoveMode;

QString QcharToString(QChar c);

template<class... Ts, int Size = sizeof...(Ts)>
static bool charIsOneOf(char c, Ts&&... args)
{
    static_assert(Size > 0, "Invalid use of charIsOne");
    return ((c == args) || ...);
}
class CTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    template<class... Ts>
    CTextEdit(Ts&&... args) : QPlainTextEdit(std::forward<Ts>(args)...){
        SetUpShortcuts();
        SetUpLineNumberAreaCode();
    }
    void keyPressEvent(QKeyEvent*) override;
    void handleEnterKey();
    //void addShortcut(QKeySequence&&,void(CTextEdit::*func)()); - doesnt work

public: //variables
void setCompleter(QCompleter *completer);
bool isPythonFile = false;

private: //variables
    QCompleter* c = nullptr;

private:
    template<class... Ts>
    void insertAllPlainText(Ts&&... args)
    {
        (this->insertPlainText(std::forward<Ts>(args)), ...);
    }

    QTextCursor moveCursor(QTextCursor::MoveOperation, QTextCursor::MoveMode = QTextCursor::MoveMode::MoveAnchor);




    int tabAmount = 2;
    QString tab = QString(tabAmount, ' ');

    char lastKeyPressed = Qt::Key_0;
    void DisplayMessage(const QString&);    
    int currentLineIndentAmount();
    int distanceFromStartOfLine();
    void SetUpShortcuts();


    QVector<QShortcut*> shortcuts;
    void duplicateLine();



    //completer code

    void insertCompletion(const QString&);
    QCompleter* completer() const;
    QString textUnderCursor() const;
    void focusInEvent(QFocusEvent*) override;

    void SetUpLineNumberAreaCode();


    //Line Number Area Code
public:
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
private:
    QWidget* lineNumberArea;
};

#endif // CTEXTEDIT_H
