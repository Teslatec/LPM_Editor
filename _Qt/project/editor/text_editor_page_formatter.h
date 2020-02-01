#ifndef TEXT_EDITOR_PAGE_FORMATTER_H
#define TEXT_EDITOR_PAGE_FORMATTER_H

#include "lpm_unicode_display.h"
#include "text_editor_page_map.h"

typedef struct TextEditorPageFormatter
{
    LPM_UnicodeDisplay * oDevice;
    TextEditorLineTable prevLineTable;
    Unicode_Buf lineBuffer;
} TextEditorPageFormatter;

void TextEditorPageFormatter_init( TextEditorPageFormatter * obj,
                                   LPM_UnicodeDisplay * oDevice,
                                   const TextEditorLineTable * prevLineTable,
                                   const Unicode_Buf * lineBuffer );
void TextEditorPageFormatter_updateCursor( TextEditorPageFormatter * obj,
                                           const LPM_DisplayCursor * cursor );
void TextEditorPageFormatter_updateText( TextEditorPageFormatter * obj,
                                         const Unicode_Buf * text,
                                         const TextEditorLineMap * updatedLineTable );

#endif // TEXT_EDITOR_PAGE_FORMATTER_H
