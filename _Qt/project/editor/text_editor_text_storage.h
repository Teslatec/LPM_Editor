#ifndef TEXT_EDITOR_TEXT_STORAGE_H
#define TEXT_EDITOR_TEXT_STORAGE_H

#include "lpm_unicode.h"

typedef struct TextEditorTextStorage
{
    Unicode_Buf textBuffer;
    size_t endOfText;
} TextEditorTextStorage;

void TextEditorTextStorage_init( TextEditorTextStorage * o,
                                 const Unicode_Buf * textBuffer );

void TextEditorTextStorage_append( TextEditorTextStorage * o,
                                   const Unicode_Buf * text );
void TextEditorTextStorage_insert( TextEditorTextStorage * o,
                                   const Unicode_Buf * text,
                                   size_t pos );
void TextEditorTextStorage_replace( TextEditorTextStorage * o,
                                    const Unicode_Buf * text,
                                    size_t pos );
void TextEditorTextStorage_remove( TextEditorTextStorage * o,
                                   size_t pos,
                                   size_t len );
void TextEditorTextStorage_truncate( TextEditorTextStorage * o,
                                     size_t pos );

void TextEditorTextStorage_read( TextEditorTextStorage * o,
                                 size_t readPosition,
                                 Unicode_Buf * readTextBuffer );

void TextEditorTextStorage_sync(TextEditorTextStorage * o);

inline size_t TextEditorTextStorage_endOfText(const TextEditorTextStorage * o)
{
    return o->endOfText;
}

inline size_t TextEditorTextStorage_freeSize(const TextEditorTextStorage * o)
{
    return o->textBuffer.size - o->endOfText;
}

#endif // TEXT_EDITOR_TEXT_STORAGE_H
