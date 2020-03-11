#ifndef MODULES_H
#define MODULES_H

#include "lpm_unicode.h"
#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"

struct Core;
struct CmdReader;
struct PageFormatter;
struct TextBuffer;
struct TextStorage;
struct TextOperator;
struct ScreenPainter;

typedef struct Modules
{
    Unicode_Buf lineBuffer;
    LPM_UnicodeKeyboard     * keyboard;
    LPM_UnicodeDisplay      * display;
    struct Core             * core;
    struct CmdReader        * cmdReader;
    struct PageFormatter    * pageFormatter;
    struct TextBuffer   * clipboardTextBuffer;
    struct TextBuffer   * undoTextBuffer;
    struct TextStorage  * textStorage;
    struct TextOperator * textOperator;
    struct ScreenPainter    * screenPainter;
} Modules;

#endif // MODULES_H
