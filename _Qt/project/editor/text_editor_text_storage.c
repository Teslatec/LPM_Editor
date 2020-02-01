#include "text_editor_text_storage.h"
#include <string.h>

static size_t _calcEndOfTextPosition(TextEditorTextStorage * o);
static void _markEndOfText(TextEditorTextStorage * o);


void TextEditorTextStorage_init( TextEditorTextStorage * o,
                                 const Unicode_Buf * textBuffer )
{
    o->textBuffer.data = textBuffer->data;
    o->textBuffer.size = textBuffer->size;
    o->endOfText = _calcEndOfTextPosition(o);
}

void TextEditorTextStorage_append( TextEditorTextStorage * o,
                                   const Unicode_Buf * text )
{
    memcpy( o->textBuffer.data + o->endOfText,
            text->data, text->size );
    o->endOfText += text->size;
    _markEndOfText(o);
}

void TextEditorTextStorage_insert( TextEditorTextStorage * o,
                                   const Unicode_Buf * text,
                                   size_t pos )
{
    memmove( o->textBuffer.data + pos + text->size,
             o->textBuffer.data + pos, text->size );
    memcpy(o->textBuffer.data + pos, text->data, text->size);
    o->endOfText += text->size;
    _markEndOfText(o);
}

void TextEditorTextStorage_replace( TextEditorTextStorage * o,
                                    const Unicode_Buf * text,
                                    size_t pos )
{
    memcpy(o->textBuffer.data + pos, text->data, text->size);
}

void TextEditorTextStorage_remove( TextEditorTextStorage * o,
                                   size_t pos,
                                   size_t len )
{
    memmove( o->textBuffer.data + pos,
             o->textBuffer.data + pos + len,
             o->endOfText - pos + len );
    o->endOfText -= len;
    _markEndOfText(o);
}

void TextEditorTextStorage_truncate( TextEditorTextStorage * o,
                                     size_t pos )
{
    o->endOfText = pos;
    _markEndOfText(o);
}

void TextEditorTextStorage_read( TextEditorTextStorage * o,
                                 size_t readPosition,
                                 Unicode_Buf * readTextBuffer )
{
    memcpy( readTextBuffer->data,
            o->textBuffer.data + readPosition,
            readTextBuffer->size * sizeof(unicode_t) );
}


size_t _calcEndOfTextPosition(TextEditorTextStorage * o)
{
    const unicode_t * begin = o->textBuffer.data;
    const unicode_t * end   = o->textBuffer.data + o->textBuffer.size;
    const unicode_t * ptr;
    for(ptr = begin; ptr != end; ptr++)
        if(*ptr == 0x0000)
            break;
    return ptr - begin;
}

void _markEndOfText(TextEditorTextStorage * o)
{
    if(o->endOfText < o->textBuffer.size)
        o->textBuffer.data[o->endOfText] = 0x0000;
}
