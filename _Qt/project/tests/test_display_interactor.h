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
                                    QObject * parent = nullptr );
    void clear();
    void write(QString data, QPoint point);
    void writeLine(QString line, int index, int bs, int s, int as);

    QString toString() const;

public slots:
    void onSetDisplayLatencyEnabled(bool state);

signals:
    void _htmlTextChanged(QString html);
    void _htmlTextUpdated();
    void _lineUpdated(int lineIndex);
    void _resetLinesUpdating();

private:
    TestDisplayViewModel vm;
    bool latencyEnabled;
    QStringList html;

    void waitForGuiRepaint();    
};

#endif // TEST_DISPLAY_INTERACTOR_H
