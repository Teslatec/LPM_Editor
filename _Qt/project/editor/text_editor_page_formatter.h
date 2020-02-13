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
} TextEditorPageFormatter;

void TextEditorPageFormatter_init(TextEditorPageFormatter * o, const TextEditorModules * modules);
void TextEditorPageFormatter_buildFirstPage(TextEditorPageFormatter * o);
void TextEditorPageFormatter_buildNextPage(TextEditorPageFormatter * o);
void TextEditorPageFormatter_buildPreviousPage(TextEditorPageFormatter * o);
void TextEditorPageFormatter_rebuildCurrentPage( TextEditorPageFormatter * o,
                                                 LPM_SelectionCursor * changedTextArea );

void TextEditorPageFormatter_updateDisplay( TextEditorPageFormatter * o,
                                            const LPM_DisplayCursor * cursor );

//void TextEditorPageFormatter_modifyCursorsWhenDisplayCursorMoved( TextEditorPageFormatter * o,
//                                                                  uint16_t moveFlags );
//void TextEditorPageFormatter_modifyCursorsWhenTextChanged( TextEditorPageFormatter * o,
//                                                            );

inline uint32_t TextEditorPageFormatter_getLineChangedFlags(TextEditorPageFormatter * o)
{
    return o->lineChangedFlags;
}
//const LPM_SelectionCursor * TextEditorPageFormatter_getSelectionCursor(TextEditorPageFormatter * o);
//const LPM_DisplayCursor * TextEditorPageFormatter_getDisplayCursor(TextEditorPageFormatter * o);

#endif // TEXT_EDITOR_PAGE_FORMATTER_H
