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
    TextEditorCursor cursorBefore;
    uint32_t enteredTextLenght;
    uint32_t deletedTextLenght;
    uint8_t flags;
} TextEditorActionStorage;

void TextEditorActionStorage_init( TextEditorActionStorage * obj,
                                   const Unicode_Buf * textBuf );

void TextEditorActionStorage_saveDeletedText( TextEditorActionStorage * obj,
                                              const Unicode_Buf * deletedText,
                                              const TextEditorCursor * cursor );
void TextEditorActionStorage_saveEnteredText( TextEditorActionStorage * obj,
                                              uint32_t enteredTextLenght,
                                              const TextEditorCursor * cursor );
void TextEditorActionStorage_saveChangedText( TextEditorActionStorage * obj,
                                              const Unicode_Buf * deletedText,
                                              uint32_t enteredTextLenght,
                                              const TextEditorCursor * cursor );

inline TextEditorAction TextEditorActionStorage_getAction(TextEditorActionStorage * obj);

void TextEditorActionStorage_restoreDeletedText( TextEditorActionStorage * obj,
                                                 Unicode_Buf * deletedText,
                                                 TextEditorCursor * cursorBefore );
void TextEditorActionStorage_restoreEnteredText( TextEditorActionStorage * obj,
                                                 uint32_t * enteredTextLenght,
                                                 TextEditorCursor * cursorBefore );
void TextEditorActionStorage_restoreChangedText( TextEditorActionStorage * obj,
                                                 Unicode_Buf * deletedText,
                                                 uint32_t * enteredTextLenght,
                                                 TextEditorCursor * cursor );

#endif // TEXT_EDITOR_USER_ACTIONS_H
