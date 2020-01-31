#include "test_display.h"
#include "test_display_interactor.h"

static void writeLine( LPM_UnicodeDisplay * i,
                       const Unicode_Buf * lineBuf,
                       const LPM_Point * position );

static void setCursor( LPM_UnicodeDisplay * i,
                       const LPM_Cursor * cursor );
static void clearScreen(LPM_UnicodeDisplay * i);

static const LPM_UnicodeDisplayFxns fxns =
{
    .writeLine   = &writeLine,
    .setCursor   = &setCursor,
    .clearScreen = &clearScreen
};

void TestDisplay_init(TestDisplay * dsp, TestDisplayInteractor * itc)
{
    dsp->base =
    {
        .fxns  = &fxns,
        .error = LPM_NO_ERROR
    };
    dsp->interactor = itc;
    // ...
}

static QString unicode_line_to_string(const Unicode_Buf * buf);

void writeLine( LPM_UnicodeDisplay * i,
                const Unicode_Buf * lineBuf,
                const LPM_Point * position )
{
    ((TestDisplay*)i)->interactor->writeLine(
                unicode_line_to_string(lineBuf),
                QPoint(position->x, position->y) );
}

void setCursor( LPM_UnicodeDisplay * i,
                const LPM_Cursor * cursor )
{
    ((TestDisplay*)i)->interactor->setCursor(
                QPoint(cursor->begin.x, cursor->begin.y),
                QPoint(cursor->end.x, cursor->end.y) );
}

void clearScreen(LPM_UnicodeDisplay * i) { (void)i; }

QString unicode_line_to_string(const Unicode_Buf * buf)
{
    QString r(buf->size, 0);
    auto data = buf->data;
    for(auto & it : r)
        it = QChar(*data++);
    return r;
}
