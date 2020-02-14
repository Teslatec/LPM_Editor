#ifndef TEXT_EDITOR_PAGE_FORMATTER_H
#define TEXT_EDITOR_PAGE_FORMATTER_H

#include "lpm_structs.h"
#include "lpm_unicode.h"
#include "text_editor_modules.h"

#define TEXT_EDITOR_PAGE_LINE_AMOUNT 16
#define TEXT_EDITOR_PAGE_CHAR_AMOUNT 64
#define TEXT_EDITOR_PAGE_GROUP_AMOUNT 4
#define TEXT_EDITOR_PAGES_IN_GROUP  4

typedef struct TextEditorLineMap
{
    uint8_t fullLen;
    uint8_t payloadLen;
    uint8_t restLen;
} TextEditorLineMap;

typedef struct TextEditorPageFormatter
{
    const TextEditorModules * modules;
    size_t pageGroupOffset[TEXT_EDITOR_PAGE_GROUP_AMOUNT];
    size_t currPageOffset;
    size_t currGroupIndex;
    size_t currPageIndex;
    TextEditorLineMap lineMap[TEXT_EDITOR_PAGE_LINE_AMOUNT];
    TextEditorLineMap prevPageLastLineMap;
    uint32_t lineChangedFlags;
    bool lastPageReached;
    LPM_DisplayCursor displayCursor;
} TextEditorPageFormatter;

void TextEditorPageFormatter_init
        ( TextEditorPageFormatter * o,
          const TextEditorModules * modules );

void TextEditorPageFormatter_setPageAtTextPosition
        ( TextEditorPageFormatter * o,
          size_t pos );

void TextEditorPageFormatter_updatePageByTextChanging
        ( TextEditorPageFormatter * o,
          const LPM_SelectionCursor * changedTextArea,
          size_t newTextPosition );

void TextEditorPageFormatter_updatePageByDisplayCursorChanging
        ( TextEditorPageFormatter * o,
          LPM_SelectionCursor * textArea
          /*Команды!!!*/ );

void TextEditorPageFormatter_updateDisplay
        ( TextEditorPageFormatter * o );

#endif // TEXT_EDITOR_PAGE_FORMATTER_H
