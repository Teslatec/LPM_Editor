#include "test_display.h"
#include "test_display_interactor.h"

static void writeLine( LPM_UnicodeDisplay * i,
                       const Unicode_Buf * lineBuf,
                       const LPM_UnicodeDisplayLineAttr * lineAttr );

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
                const Unicode_Buf * lineBuf,
                const LPM_UnicodeDisplayLineAttr * lineAttr )
{
    ((TestDisplay*)i)->interactor->writeLine(
                unicode_line_to_string(lineBuf), lineAttr->index,
                lineAttr->lenBeforeSelect, lineAttr->lenSelect,
                lineAttr->lenAfterSelect);
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
