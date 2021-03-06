#ifndef TEST_DISPLAY_HTML_CONVERTOR_H
#define TEST_DISPLAY_HTML_CONVERTOR_H

#include <QStringList>
#include <QPoint>

class TestDisplayHtmlConvertor
{
public:
    static QString convert( const QStringList & src,
                            QPoint cursorBegin,
                            QPoint cursorEnd );

    static QString convertLine(const QString & line, const QPoint & curs, bool selectAreaUnderlined);
};

#endif // TEST_DISPLAY_HTML_CONVERTOR_H
