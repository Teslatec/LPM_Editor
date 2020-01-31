#ifndef LPM_UNICODE_DISPLAY_H
#define LPM_UNICODE_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "lpm_error.h"
#include "lpm_structs.h"
#include "lpm_unicode.h"

struct LPM_UnicodeDisplay;

typedef struct LPM_UnicodeDisplayFxns
{
    void (*writeLine)   ( struct LPM_UnicodeDisplay * i,
                          const Unicode_Buf * lineBuf,
                          const LPM_Point * position );    // у - индекс строки, в которую выводим, x - смещение точки ввода от левого края
    void (*setCursor)   ( struct LPM_UnicodeDisplay * i,
                          const LPM_Cursor * cursor );
    void (*clearScreen) (struct LPM_UnicodeDisplay * i);
} LPM_UnicodeDisplayFxns;

typedef struct LPM_UnicodeDisplay
{
    const LPM_UnicodeDisplayFxns * fxns;
    error_t error;
} LPM_UnicodeDisplay;

inline void LPM_UnicodeDisplay_writeLine( LPM_UnicodeDisplay * i,
                                          const Unicode_Buf * lineBuf,
                                          const LPM_Point * position )
{
    (*(i->fxns->writeLine))(i, lineBuf, position);
}

inline void LPM_UnicodeDisplay_setCursor( LPM_UnicodeDisplay * i,
                                          const LPM_Cursor * cursor )
{
    (*(i->fxns->setCursor))(i, cursor);
}

inline void LPM_UnicodeDisplay_clearScreen(LPM_UnicodeDisplay * i)
{
    (*(i->fxns->clearScreen))(i);
}

inline bool LPM_UnicodeDisplay_errorOccured(LPM_UnicodeDisplay * i)
{
    return LPM_ErrorOccured(i->error);
}

#endif // LPM_UNICODE_DISPLAY_H
