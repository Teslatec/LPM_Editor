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
    void _lineUpdated(int lineIndex);
    void _resetLinesUpdating();
    void _setDisplayLatencyEnabled(bool state);

public slots:
    void start(QTextEdit * textEdit, bool displayLatencyEnabled);
    void gui_key_event(QKeyEvent * evt);
    void gui_set_display_latency_enabled(bool state);
};

#endif // TEST_TEXT_EDITOR_H
