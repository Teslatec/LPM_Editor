#ifndef TEXT_EDITOR_MODULES_H
#define TEXT_EDITOR_MODULES_H

#include "lpm_unicode.h"
#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"

struct TextEditorCore;
struct TextEditorCmdReader;
struct TextEditorTextOperator;
struct TextEditorTextStorage;
struct TextEditorUserActions;
struct TextEditorClipboard;

typedef struct TextEditorModules
{
    Unicode_Buf keyboardBuffer;
    Unicode_Buf lineBuffer;
    Unicode_Buf actionsBuffer;
    Unicode_Buf clipboardBuffer;
    Unicode_Buf textBuffer;
    LPM_UnicodeKeyboard * keyboard;
    LPM_UnicodeDisplay  * display;
    struct TextEditorCore          * core;
    struct TextEditorCmdReader     * cmdReader;
    struct TextEditorTextOperator  * textOperator;
    struct TextEditorTextStorage   * textStorage;
    struct TextEditorActionStorage * actionStorage;
    struct TextEditorClipboard     * clipboard;
} TextEditorModules;

#endif // TEXT_EDITOR_MODULES_H
