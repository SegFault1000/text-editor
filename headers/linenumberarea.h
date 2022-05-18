#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H
#include <QWidget>
#include <QPlainTextEdit>
#include <QPainter>
class CTextEdit;
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CTextEdit* editor);

    QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CTextEdit* textEdit;
};

#endif // LINENUMBERAREA_H
