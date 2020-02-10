#ifndef TEXT_EDITOR_TEXT_OPERATOR_H
#define TEXT_EDITOR_TEXT_OPERATOR_H

#include "lpm_structs.h"
#include "text_editor_text_storage.h"

typedef struct TextEditorTextOperator
{
    TextEditorTextStorage * textStorage;
} TextEditorTextOperator;

bool TextEditorTextOperator_removeAndWrite( TextEditorTextOperator * o,
                                            LPM_SelectionCursor * removingArea,
                                            const Unicode_Buf * textToWrite );

void TextEditorTextOperator_read( TextEditorTextOperator * o,
                                  size_t readPosition,
                                  Unicode_Buf * readTextBuffer );


inline void TextEditorTextOperator_init( TextEditorTextOperator * o,
                                         TextEditorTextStorage * storage)
{
    o->textStorage = storage;
}

inline void TextEditorTextOperator_sync(TextEditorTextOperator * o)
{
    return TextEditorTextStorage_sync(o->textStorage);
}

inline size_t TextEditorTextOperator_freeSize(TextEditorTextOperator * o)
{
    return TextEditorTextStorage_freeSize(o->textStorage);
}

#endif // TEXT_EDITOR_TEXT_OPERATOR_H
