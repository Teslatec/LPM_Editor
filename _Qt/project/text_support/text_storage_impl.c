#include "text_storage_impl.h"
#include <string.h>

static size_t _calcEndOfTextPosition(TextStorageImpl * o);
static void _markEndOfText(TextStorageImpl * o);

void TextStorageImpl_init(TextStorageImpl * o, Unicode_Buf * textBuffer)
{
    o->textBuffer = textBuffer;
    o->endOfText  = _calcEndOfTextPosition(o);
}

void TextStorageImpl_append(TextStorageImpl * o, const Unicode_Buf * text)
{
    memcpy( o->textBuffer->data + o->endOfText,
            text->data, text->size * sizeof(unicode_t) );
    o->endOfText += text->size;
    _markEndOfText(o);
}

void TextStorageImpl_insert(TextStorageImpl * o, const Unicode_Buf * text, size_t pos)
{
    memmove( o->textBuffer->data + pos + text->size,
             o->textBuffer->data + pos, (o->endOfText - pos) * sizeof(unicode_t) );
    memcpy(o->textBuffer->data + pos, text->data, text->size * sizeof(unicode_t) );
    o->endOfText += text->size;
    _markEndOfText(o);
}

void TextStorageImpl_replace(TextStorageImpl * o, const Unicode_Buf * text, size_t pos)
{
    memcpy(o->textBuffer->data + pos, text->data, text->size * sizeof(unicode_t));
}

void TextStorageImpl_remove(TextStorageImpl * o, size_t pos, size_t len)
{
    memmove( o->textBuffer->data + pos,
             o->textBuffer->data + pos + len,
             (o->endOfText - pos + len) * sizeof(unicode_t));
    o->endOfText -= len;
    _markEndOfText(o);
}

void TextStorageImpl_truncate(TextStorageImpl * o, size_t pos)
{
    o->endOfText = pos;
    _markEndOfText(o);
}

void TextStorageImpl_read(TextStorageImpl * o, size_t readPosition, Unicode_Buf * readTextBuffer)
{
    memcpy( readTextBuffer->data,
            o->textBuffer->data + readPosition,
            readTextBuffer->size * sizeof(unicode_t) );
}

void TextStorageImpl_sync(TextStorageImpl * o)
{
    (void)o;
}


size_t _calcEndOfTextPosition(TextStorageImpl * o)
{
    const unicode_t * begin = o->textBuffer->data;
    const unicode_t * end   = o->textBuffer->data + o->textBuffer->size;
    const unicode_t * ptr;
    for(ptr = begin; ptr != end; ptr++)
        if(*ptr == 0x0000)
            break;
    return ptr - begin;
}

void _markEndOfText(TextStorageImpl * o)
{
    if(o->endOfText < o->textBuffer->size)
        o->textBuffer->data[o->endOfText] = 0x0000;
}
