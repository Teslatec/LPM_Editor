#ifndef MODULES_H
#define MODULES_H

#include "lpm_unicode.h"
#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"

struct Core;
struct CmdReader;
struct PageFormatter;
struct LPM_TextBuffer;
struct LPM_TextStorage;
struct LPM_TextOperator;
struct TextStorageImpl;
struct LPM_Lang;
struct ScreenPainter;

typedef struct Modules
{
    Unicode_Buf lineBuffer;
    LPM_UnicodeKeyboard     * keyboard;
    LPM_UnicodeDisplay      * display;
    struct Core             * core;
    struct CmdReader        * cmdReader;
    struct PageFormatter    * pageFormatter;
    struct LPM_TextBuffer   * clipboardTextBuffer;
    struct LPM_TextBuffer   * undoTextBuffer;
    struct LPM_TextStorage  * textStorage;
    struct TextStorageImpl  * textStorageImpl;
    struct LPM_TextOperator * textOperator;
    struct LPM_Lang         * lang;
    struct ScreenPainter    * screenPainter;
} Modules;

#endif // MODULES_H
