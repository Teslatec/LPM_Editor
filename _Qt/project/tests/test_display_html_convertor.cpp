#include "test_display_html_convertor.h"

#include <QDebug>

const QString htmlLineFeed          = "<br>";
const QString htmlCursorBegin       = "<u>";
const QString htmlCursorEnd         = "</u>";
const QString htmlHighlightingBegin = "<font color=\"#8852C5\"><b>";
const QString htmlHighlightingEnd   = "</b></font>";

QString convertTextWithoutHighlighting( const QStringList & src,
                                        QPoint cursor );
QString convertTextWithOneLineHighlighted( const QStringList & src,
                                           QPoint cursorBegin,
                                           QPoint cursorEnd );
QString convertTextWithMultiLineHighlighted( const QStringList & src,
                                             QPoint cursorBegin,
                                             QPoint cursorEnd );

QString makeLine(const QString & line);
QString makeLineWithCursor(const QString & line, int x);
QString makeLineWithHighlighting(const QString & line, int xBegin, int xEnd);
QString makeLineWithHighlightingBegin(const QString & line, int x);
QString makeLineWithHighlightingEnd(const QString & line, int x);

QString TestDisplayHtmlConvertor::convert( const QStringList & src,
                                           QPoint cursorBegin,
                                           QPoint cursorEnd )
{
    if(cursorBegin == cursorEnd)
        return convertTextWithoutHighlighting(src, cursorBegin);

    if(cursorBegin.y() == cursorEnd.y())
        return convertTextWithOneLineHighlighted(src, cursorBegin, cursorEnd);

    return convertTextWithMultiLineHighlighted(src, cursorBegin, cursorEnd);
}

QString convertTextWithoutHighlighting( const QStringList & src,
                                        QPoint cursor )
{
    QString html;
    int lineIndex = 0;
    for(auto line : src)
    {
        if(lineIndex == cursor.y())
            html += makeLineWithCursor(line, cursor.x());
        else
            html += makeLine(line);
        lineIndex++;
    }
    return html;
}

QString convertTextWithOneLineHighlighted( const QStringList & src,
                                           QPoint cursorBegin,
                                           QPoint cursorEnd )
{
    QString html;
    int lineIndex = 0;
    for(auto line : src)
    {
        if(lineIndex == cursorBegin.y())
            html += makeLineWithHighlighting(line, cursorBegin.x(), cursorEnd.x());
        else
            html += makeLine(line);
        lineIndex++;
    }
    return html;
}

QString convertTextWithMultiLineHighlighted( const QStringList & src,
                                             QPoint cursorBegin,
                                             QPoint cursorEnd )
{
    QString html;
    int lineIndex = 0;
    for(auto line : src)
    {
        if(lineIndex == cursorBegin.y())
            html += makeLineWithHighlightingBegin(line, cursorBegin.x());
        else if(lineIndex == cursorEnd.y())
            html += makeLineWithHighlightingEnd(line, cursorEnd.x());
        else
            html += makeLine(line);
        lineIndex++;
    }
    return html;
}

QString makeLine(const QString & line)
{
    return line + htmlLineFeed;
}

QString makeLineWithCursor(const QString & line, int x)
{
    QString html = line;
    html.insert(x+1, htmlCursorEnd);
    html.insert(x, htmlCursorBegin);
    html += htmlLineFeed;
    return html;
}

QString makeLineWithHighlighting(const QString & line, int xBegin, int xEnd)
{
    QString html = line;
    html.insert(xEnd, htmlHighlightingEnd);
    html.insert(xBegin, htmlHighlightingBegin);
    html += htmlLineFeed;
    return html;
}


QString makeLineWithHighlightingBegin(const QString & line, int x)
{
    QString html = line;
    html.insert(x, htmlHighlightingBegin);
    html += htmlLineFeed;
    return html;
}

QString makeLineWithHighlightingEnd(const QString & line, int x)
{
    QString html = line;
    html.insert(x, htmlHighlightingEnd);
    html += htmlLineFeed;
    return html;
}

