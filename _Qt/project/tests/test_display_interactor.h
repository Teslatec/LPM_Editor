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
                                    QObject * parent = nullptr );
    void clear();
    void write(QString data, QPoint point);
    void writeLine(QString line, QPoint pos);
    void setCursor(QPoint begin, QPoint end);

    QString toString() const;

signals:
    void _htmlTextChanged(QString html);
    void _htmlTextUpdated();

private:
    TestDisplayViewModel vm;

    void waitForGuiRepaint();
};

#endif // TEST_DISPLAY_INTERACTOR_H
