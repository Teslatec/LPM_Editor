#include "test_display.h"
#include "test_display_interactor.h"

static void writeLine( LPM_UnicodeDisplay * i,
                       size_t index,
                       const Unicode_Buf * line,
                       const LPM_SelectionCursor * curs );

static void clearScreen(LPM_UnicodeDisplay * i);

static const LPM_UnicodeDisplayFxns fxns =
{
    .writeLine   = &writeLine,
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
                size_t index,
                const Unicode_Buf * line,
                const LPM_SelectionCursor * curs )
{
    ((TestDisplay*)i)->interactor->writeLine( index,
                                              unicode_line_to_string(line),
                                              QPoint(curs->pos, curs->len) );
}

void clearScreen(LPM_UnicodeDisplay * i)
{
    ((TestDisplay*)i)->interactor->clear();
}

QString unicode_line_to_string(const Unicode_Buf * buf)
{
    QString r(buf->size, 0);
    auto data = buf->data;
    for(auto & it : r)
        it = QChar(*data++);
    return r;
}
