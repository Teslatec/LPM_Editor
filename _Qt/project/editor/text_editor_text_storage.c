#include "text_editor_text_storage.h"
#include <string.h>

static size_t _calcEndOfTextPosition(TextEditorTextStorage * o);


void TextEditorTextStorage_init( TextEditorTextStorage * o,
                                 const Unicode_Buf * textBuffer )
{
    o->textBuffer.data = textBuffer->data;
    o->textBuffer.size = textBuffer->size;
    o->endOfText = _calcEndOfTextPosition(o);
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
