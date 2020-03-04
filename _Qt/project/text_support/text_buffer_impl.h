#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include "lpm_unicode.h"

typedef struct TextBufferImpl
{
    Unicode_Buf * textBuffer;
    size_t endOfText;
} TextBufferImpl;

void TextBufferImpl_init(TextBufferImpl * o, Unicode_Buf * textBuffer);
void TextBufferImpl_append(TextBufferImpl * o, const Unicode_Buf * text );
void TextBufferImpl_insert(TextBufferImpl * o, const Unicode_Buf * text, size_t pos);
void TextBufferImpl_replace(TextBufferImpl * o, const Unicode_Buf * text, size_t pos );
void TextBufferImpl_remove(TextBufferImpl * o, size_t pos, size_t len);
void TextBufferImpl_truncate(TextBufferImpl * o, size_t pos);
void TextBufferImpl_read(TextBufferImpl * o, size_t readPosition, Unicode_Buf * readTextBuffer);
void TextBufferImpl_sync(TextBufferImpl * o);

static inline size_t TextBufferImpl_endOfText(const TextBufferImpl * o)
{
    return o->endOfText;
}

static inline size_t TextBufferImpl_freeSize(const TextBufferImpl * o)
{
    return o->textBuffer->size - o->endOfText;
}

#endif // TEXT_BUFFER_H
