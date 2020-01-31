#ifndef TEST_DISPLAY_VIEW_MODEL_H
#define TEST_DISPLAY_VIEW_MODEL_H

#include <QPoint>
#include <QStringList>

struct TestDisplayViewModel
{
    TestDisplayViewModel( int symbolAmount_ = 0,
                          int lineAmount_ = 0)
        : symbolAmount(symbolAmount_)
        , lineAmount(lineAmount_)
        , cursorBegin(0,0)
        , cursorEnd(0,0)
        , data()
    {
        for(int i = 0; i < lineAmount_; i++)
            data.append(QString(symbolAmount_, QChar(0x2591)/*' '*/));
    }

    QString toString() const
    {
        QString r;
        for(auto it : data)
            r += it + QChar::LineFeed;
        return r;
    }


    int symbolAmount;
    int lineAmount;
    QPoint cursorBegin;
    QPoint cursorEnd;
    QStringList data;
};

#endif // TEST_DISPLAY_VIEW_MODEL_H
