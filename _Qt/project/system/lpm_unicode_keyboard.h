#ifndef LPM_UNICODE_KEYBOARD_H
#define LPM_UNICODE_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

#include "lpm_error.h"
#include "lpm_unicode.h"

struct LPM_UnicodeKeyboard;

typedef struct LPM_UnicodeKeyboardFxns
{
    void (*read)( struct LPM_UnicodeKeyboard * i,
                  Unicode_Buf * buf,
                  uint32_t timeoutMs );
} LPM_UnicodeKeyboardFxns;

typedef struct LPM_UnicodeKeyboard
{
    const LPM_UnicodeKeyboardFxns * fxns;
    error_t error;
} LPM_UnicodeKeyboard;


inline void LPM_UnicodeKeyboard_read( LPM_UnicodeKeyboard * i,
                                      Unicode_Buf * buf,
                                      uint32_t timeoutMs )
{
    (*(i->fxns->read))(i, buf, timeoutMs);
}

inline bool LPM_UnicodeKeyboard_errorOccured(LPM_UnicodeKeyboard * i)
{
    return LPM_ErrorOccured(i->error);
}

#endif // LPM_UNICODE_KEYBOARD_H
