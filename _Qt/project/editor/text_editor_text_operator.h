#ifndef TEXT_EDITOR_TEXT_OPERATOR_H
#define TEXT_EDITOR_TEXT_OPERATOR_H

#include "lpm_unicode.h"

typedef struct TextEditorTextOperator
{
    Unicode_Buf textBuffer;
    size_t endOfText;
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
    return o->textBuffer.size - o->endOfText;
}

//void TextEditorTextOperator_insert( TextEditorTextOperator * obj,
//                                    const Unicode_Buf * buf );
//void TextEditorTextOperator_replace( TextEditorTextOperator * obj,
//                                     const Unicode_Buf * buf );
//void TextEditorTextOperator_remove( TextEditorTextOperator * obj,
//                                    uint32_t pos );

//unicode_t TextEditorTextOperator_readChar(TextEditorTextOperator * obj);
//void TextEditorTextOperator_readBuffer(TextEditorTextOperator * obj, Unicode_Buf * textBuffer);

//const unicode_t * TextEditorTextOperator_data( TextEditorTextOperator * obj,
//                                               uint32_t pos );

//size_t TextEditorTextOperator_textSize(TextEditorTextOperator * obj);

//void TextEditorTextOperator_sync(TextEditorTextOperator * obj);


#endif // TEXT_EDITOR_TEXT_OPERATOR_H
