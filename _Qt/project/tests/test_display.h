#ifndef TEST_DISPLAY_H
#define TEST_DISPLAY_H

#include <QObject>
#include <QPoint>

#include "lpm_unicode_display.h"

class TestDisplayInteractor;

struct TestDisplay
{
    LPM_UnicodeDisplay base;
    TestDisplayInteractor * interactor;
};

void TestDisplay_init(TestDisplay * dsp, TestDisplayInteractor * itc);

inline LPM_UnicodeDisplay * TestDisplay_base(TestDisplay * dsp)
{
    return &dsp->base;
}

#endif // TEST_DISPLAY_H
