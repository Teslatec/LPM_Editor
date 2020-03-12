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
struct TextStorageImpl;
struct TextOperator;
struct ScreenPainter;
struct LPM_LangFxns;
struct LPM_EncodingFxns;
struct LPM_MeteoFxns;

typedef struct Modules
{
    Unicode_Buf lineBuffer;
    Unicode_Buf charBuffer;
    struct Core             * core;
    struct CmdReader        * cmdReader;
    struct PageFormatter    * pageFormatter;
    struct TextBuffer       * clipboardTextBuffer;
    struct TextBuffer       * undoTextBuffer;
    struct TextStorage      * textStorage;
    struct TextStorageImpl  * textStorageImpl;
    struct TextOperator     * textOperator;
    struct ScreenPainter    * screenPainter;
    struct LPM_LangFxns     * langFxns;
    struct LPM_EncodingFxns * encodingFxns;
    struct LPM_MeteoFxns    * meteoFxns;
} Modules;

#define MODULES_AMOUNT 12
#define POINTER_SIZE   4

#endif // MODULES_H
