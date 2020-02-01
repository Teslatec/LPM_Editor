#ifndef TEXT_EDITOR_USER_ACTIONS_H
#define TEXT_EDITOR_USER_ACTIONS_H

#include "lpm_unicode.h"
#include "text_editor_page_map.h"

typedef enum TextEditorAction
{
    TEXT_EDITOR_ACTION_TEXT_ENTERED,
    TEXT_EDITOR_ACTION_TEXT_DELETED,
    TEXT_EDITOR_ACTION_TEXT_CHANGED
} TextEditorAction;

typedef struct TextEditorActionStorage
{
    Unicode_Buf deletedText;
    LPM_SelectionCursor cursorBefore;
    uint32_t enteredTextLenght;
    uint32_t deletedTextLenght;
    uint8_t flags;
} TextEditorActionStorage;

void TextEditorActionStorage_init( TextEditorActionStorage * obj,
                                   const Unicode_Buf * textBuf );

void TextEditorActionStorage_saveDeletedText( TextEditorActionStorage * obj,
                                              const Unicode_Buf * deletedText,
                                              const LPM_SelectionCursor * cursor );
void TextEditorActionStorage_saveEnteredText( TextEditorActionStorage * obj,
                                              uint32_t enteredTextLenght,
                                              const LPM_SelectionCursor * cursor );
void TextEditorActionStorage_saveChangedText( TextEditorActionStorage * obj,
                                              const Unicode_Buf * deletedText,
                                              uint32_t enteredTextLenght,
                                              const LPM_SelectionCursor * cursor );

inline TextEditorAction TextEditorActionStorage_getAction(TextEditorActionStorage * obj);

void TextEditorActionStorage_restoreDeletedText( TextEditorActionStorage * obj,
                                                 Unicode_Buf * deletedText,
                                                 LPM_SelectionCursor * cursorBefore );
void TextEditorActionStorage_restoreEnteredText( TextEditorActionStorage * obj,
                                                 uint32_t * enteredTextLenght,
                                                 LPM_SelectionCursor * cursorBefore );
void TextEditorActionStorage_restoreChangedText( TextEditorActionStorage * obj,
                                                 Unicode_Buf * deletedText,
                                                 uint32_t * enteredTextLenght,
                                                 LPM_SelectionCursor * cursor );

#endif // TEXT_EDITOR_USER_ACTIONS_H
