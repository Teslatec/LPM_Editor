#ifndef LPM_UNICODE_H
#define LPM_UNICODE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint16_t unicode_t;

typedef enum Unicode_CtrlSym
{
    UNICODE_ESC         = 0xE01B,
    UNICODE_BACKSPACE   = 0xE008,
    UNICODE_TAB         = 0xE009,
    UNICODE_ENTER       = 0xE00A,
    UNICODE_DEL         = 0xE07F,

    UNICODE_CTRL_P      = 0xE101,
    UNICODE_CTRL_R      = 0xE181,
    UNICODE_ALT_P       = 0xE102,
    UNICODE_ALT_R       = 0xE182,
    UNICODE_SHIFT_P     = 0xE103,
    UNICODE_SHIFT_R     = 0xE183,

    UNICODE_LANG_N      = 0xE104,
    UNICODE_LANG_A      = 0xE184,
    UNICODE_CAPS_N      = 0xE105,
    UNICODE_CAPS_A      = 0xE185,
    UNICODE_INSERT_N    = 0xE106,
    UNICODE_INSERT_A    = 0xE186,

    UNICODE_LEFT        = 0xE108,
    UNICODE_RIGHT       = 0xE109,
    UNICODE_UP          = 0xE10A,
    UNICODE_DOWN        = 0xE10B,
    UNICODE_HOME        = 0xE10C,
    UNICODE_END         = 0xE10D,
    UNICODE_PGUP        = 0xE10E,
    UNICODE_PGDN        = 0xE10F,

    UNICODE_TIMEOUT     = 0xE200,
} Unicode_CtrlSym;

#define UNICODE_LIGHT_SHADE 0x2591

typedef enum Unicode_SymType
{
    UNICODE_SYM_TYPE_CONTROL,
    UNICODE_SYM_TYPE_BASIC_LATIN,
    UNICODE_SYM_TYPE_CYRILLIC,
    UNICODE_SYM_TYPE_DIACRITIC,
    UNICODE_SYM_TYPE_LAO,
    UNICODE_SYM_TYPE_UNSUPPORTED
} Unicode_SymType;

inline Unicode_SymType Unicode_getSymType(unicode_t sym)
{
    Unicode_SymType type;
    if((sym >= 0x0020) && (sym <= 0x007E))
        type = UNICODE_SYM_TYPE_BASIC_LATIN;
    else if((sym >= 0x0400) && (sym <= 0x04FF))
        type = UNICODE_SYM_TYPE_CYRILLIC;
    else if((sym >= 0x0300) && (sym <= 0x036F))
        type = UNICODE_SYM_TYPE_DIACRITIC;
    else if((sym >= 0xE000) && (sym <= 0xE3FF))
        type = UNICODE_SYM_TYPE_CONTROL;
    else if((sym >= 0x0E80) && (sym <= 0x0EDF))
        type = UNICODE_SYM_TYPE_LAO;
    else
        type = UNICODE_SYM_TYPE_UNSUPPORTED;
    return type;
}

inline bool Unicode_symIsCtrlSym(unicode_t sym)
{
    return (sym >= 0xE000) && (sym <= 0xE3FF);
}

typedef struct Unicode_Buf
{
    unicode_t * data;
    size_t size;
} Unicode_Buf;

#endif // LPM_UNICODE_H
