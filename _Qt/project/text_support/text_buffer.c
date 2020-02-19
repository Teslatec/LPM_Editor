#include "text_buffer.h"
#include <string.h>

static size_t _calcEndOfTextPosition(TextBuffer * o);
static void _markEndOfText(TextBuffer * o);

void TextBuffer_init(TextBuffer * o, Unicode_Buf * textBuffer)
{
    o->textBuffer = textBuffer;
    o->endOfText  = _calcEndOfTextPosition(o);
}

void TextBuffer_append(TextBuffer * o, const Unicode_Buf * text)
{
    memcpy( o->textBuffer->data + o->endOfText,
            text->data, text->size * sizeof(unicode_t) );
    o->endOfText += text->size;
    _markEndOfText(o);
}

void TextBuffer_insert(TextBuffer * o, const Unicode_Buf * text, size_t pos)
{
    memmove( o->textBuffer->data + pos + text->size,
             o->textBuffer->data + pos, (o->endOfText - pos) * sizeof(unicode_t) );
    memcpy(o->textBuffer->data + pos, text->data, text->size * sizeof(unicode_t) );
    o->endOfText += text->size;
    _markEndOfText(o);
}

void TextBuffer_replace(TextBuffer * o, const Unicode_Buf * text, size_t pos)
{
    memcpy(o->textBuffer->data + pos, text->data, text->size * sizeof(unicode_t));
}

void TextBuffer_remove(TextBuffer * o, size_t pos, size_t len)
{
    memmove( o->textBuffer->data + pos,
             o->textBuffer->data + pos + len,
             (o->endOfText - pos + len) * sizeof(unicode_t));
    o->endOfText -= len;
    _markEndOfText(o);
}

void TextBuffer_truncate(TextBuffer * o, size_t pos)
{
    o->endOfText = pos;
    _markEndOfText(o);
}

void TextBuffer_read(TextBuffer * o, size_t readPosition, Unicode_Buf * readTextBuffer)
{
    memcpy( readTextBuffer->data,
            o->textBuffer->data + readPosition,
            readTextBuffer->size * sizeof(unicode_t) );
}


size_t _calcEndOfTextPosition(TextBuffer * o)
{
    const unicode_t * begin = o->textBuffer->data;
    const unicode_t * end   = o->textBuffer->data + o->textBuffer->size;
    const unicode_t * ptr;
    for(ptr = begin; ptr != end; ptr++)
        if(*ptr == 0x0000)
            break;
    return ptr - begin;
}

void _markEndOfText(TextBuffer * o)
{
    if(o->endOfText < o->textBuffer->size)
        o->textBuffer->data[o->endOfText] = 0x0000;
}
