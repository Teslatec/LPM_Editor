#ifndef TEST_DISPLAY_INTERACTOR_H
#define TEST_DISPLAY_INTERACTOR_H

#include "test_display_view_model.h"

#include <QObject>

class TestDisplayInteractor : public QObject
{
    Q_OBJECT
public:
    explicit TestDisplayInteractor( int symbolAmount,
                                    int lineAmount,
                                    bool latencyEnabled_,
                                    bool selectAreaUnderlined_,
                                    QObject * parent = nullptr );
    void clear();
    void writeLine(int index, QString line, QPoint curs);

    QString toString() const;

public slots:
    void onSetDisplayLatencyEnabled(bool state);
    void onSetDisplaySelectAreaUnderlined(bool state);

signals:
    void _htmlTextChanged(QString html);
    void _htmlTextUpdated();
    void _lineUpdated(int lineIndex);
    void _resetLinesUpdating();

private:
    TestDisplayViewModel vm;
    bool latencyEnabled;
    bool selectAreaUnderlined;
    QStringList html;

    void waitForGuiRepaint();    
};

#endif // TEST_DISPLAY_INTERACTOR_H
