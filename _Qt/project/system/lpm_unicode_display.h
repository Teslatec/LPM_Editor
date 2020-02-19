#ifndef LPM_UNICODE_DISPLAY_H
#define LPM_UNICODE_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "lpm_error.h"
#include "lpm_structs.h"
#include "lpm_unicode.h"

struct LPM_UnicodeDisplay;

typedef struct LPM_UnicodeDisplayLineAttr
{
    uint8_t index;              // Номер строки
    uint8_t lenBeforeSelect;    // Число символов до подчеркивания
    uint8_t lenSelect;          // Число подчеркнутых символов
    uint8_t lenAfterSelect;     // Число символов после подчеркивания
} LPM_UnicodeDisplayLineAttr;

typedef struct LPM_UnicodeDisplayFxns
{
    void (*writeLine)   ( struct LPM_UnicodeDisplay * i,
                          const Unicode_Buf * lineBuf,
                          const LPM_UnicodeDisplayLineAttr * lineAttr );
    void (*clearScreen) (struct LPM_UnicodeDisplay * i);
} LPM_UnicodeDisplayFxns;

typedef struct LPM_UnicodeDisplay
{
    const LPM_UnicodeDisplayFxns * fxns;
    error_t error;
} LPM_UnicodeDisplay;

inline void LPM_UnicodeDisplay_writeLine( LPM_UnicodeDisplay * i,
                                          const Unicode_Buf * lineBuf,
                                          const LPM_UnicodeDisplayLineAttr * lineAttr )
{
    (*(i->fxns->writeLine))(i, lineBuf, lineAttr);
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
