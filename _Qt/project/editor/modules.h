#ifndef MODULES_H
#define MODULES_H

#include "lpm_unicode.h"
#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"

struct Core;
struct CmdReader;
struct ActionStorage;
struct Clipboard;
struct PageFormatter;
struct LPM_TextStorage;
struct LPM_TextOperator;
struct LPM_Lang;

typedef struct Modules
{
    Unicode_Buf keyboardBuffer;
    Unicode_Buf lineBuffer;
    Unicode_Buf actionsBuffer;
    Unicode_Buf clipboardBuffer;
    Unicode_Buf textBuffer;
    LPM_UnicodeKeyboard     * keyboard;
    LPM_UnicodeDisplay      * display;
    struct Core             * core;
    struct CmdReader        * cmdReader;
    struct PageFormatter    * pageFormatter;
    struct ActionStorage    * actionStorage;
    struct Clipboard        * clipboard;
    struct LPM_TextStorage  * textStorage;
    struct LPM_TextOperator * textOperator;
    struct LPM_Lang         * lang;
} Modules;

#endif // MODULES_H
