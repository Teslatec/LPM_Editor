#ifndef TEXT_EDITOR_TEXT_OPERATOR_H
#define TEXT_EDITOR_TEXT_OPERATOR_H

#include "text_editor_text_storage.h"

typedef struct TextEditorTextOperator
{
    TextEditorTextStorage storage;
} TextEditorTextOperator;

void TextEditorTextOperator_init( TextEditorTextOperator * o,
                                  const Unicode_Buf * textBuffer );

bool TextEditorTextOperator_removeAndWrite( TextEditorTextOperator * o,
                                            size_t removeAndWritePosition,
                                            size_t removeTextSize,
                                            const Unicode_Buf * textToWrite );

void TextEditorTextOperator_read( TextEditorTextOperator * o,
                                  size_t readPosition,
                                  Unicode_Buf * readTextBuffer );

inline size_t TextEditorTextOperator_freeSize(TextEditorTextOperator * o)
{
    return TextEditorTextStorage_freeSize(o->storage);
}

#endif // TEXT_EDITOR_TEXT_OPERATOR_H
