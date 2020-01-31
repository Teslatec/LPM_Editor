#ifndef TEST_TEXT_EDITOR_H
#define TEST_TEXT_EDITOR_H

#include <QObject>
#include <QKeyEvent>
#include <QPoint>

class QTextEdit;

class TestTextEditor : public QObject
{
    Q_OBJECT
public:
    explicit TestTextEditor(QObject * p);

signals:
    void _gotKey(int code);
    void _textUpdated();

public slots:
    void start(QTextEdit * textEdit);
    void gui_key_event(QKeyEvent * evt);
};

#endif // TEST_TEXT_EDITOR_H
