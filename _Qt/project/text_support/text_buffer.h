#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include "lpm_unicode.h"

typedef struct TextBuffer
{
    Unicode_Buf * textBuffer;
    size_t endOfText;
} TextBuffer;

void TextBuffer_init(TextBuffer * o, Unicode_Buf * textBuffer);
void TextBuffer_append(TextBuffer * o, const Unicode_Buf * text );
void TextBuffer_insert(TextBuffer * o, const Unicode_Buf * text, size_t pos);
void TextBuffer_replace(TextBuffer * o, const Unicode_Buf * text, size_t pos );
void TextBuffer_remove(TextBuffer * o, size_t pos, size_t len);
void TextBuffer_truncate(TextBuffer * o, size_t pos);
void TextBuffer_read(TextBuffer * o, size_t readPosition, Unicode_Buf * readTextBuffer);
void TextBuffer_sync(TextBuffer * o);

inline size_t TextBuffer_endOfText(const TextBuffer * o)
{
    return o->endOfText;
}

inline size_t TextBuffer_freeSize(const TextBuffer * o)
{
    return o->textBuffer->size - o->endOfText;
}

#endif // TEXT_BUFFER_H
