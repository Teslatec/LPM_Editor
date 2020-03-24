#ifndef TEXT_STORAGE_IMPL_H
#define TEXT_STORAGE_IMPL_H

#include "lpm_unicode.h"

typedef struct TextStorageImpl
{
    //Unicode_Buf * textBuffer;
    Unicode_Buf textBuffer;
    size_t endOfText;
} TextStorageImpl;

void TextStorageImpl_init(TextStorageImpl * o, const Unicode_Buf * textBuffer);
void TextStorageImpl_setMaxSize(TextStorageImpl * o, size_t maxSize);
void TextStorageImpl_recalcEndOfText(TextStorageImpl * o);
void TextStorageImpl_clear(TextStorageImpl * o, bool deep);
void TextStorageImpl_append(TextStorageImpl * o, const Unicode_Buf * text );
void TextStorageImpl_insert(TextStorageImpl * o, const Unicode_Buf * text, size_t pos);
void TextStorageImpl_replace(TextStorageImpl * o, const Unicode_Buf * text, size_t pos );
void TextStorageImpl_remove(TextStorageImpl * o, size_t pos, size_t len);
void TextStorageImpl_truncate(TextStorageImpl * o, size_t pos);
void TextStorageImpl_read(TextStorageImpl * o, size_t readPosition, Unicode_Buf * readTextBuffer);
void TextStorageImpl_sync(TextStorageImpl * o);

static inline size_t TextStorageImpl_endOfText(const TextStorageImpl * o)
{
    return o->endOfText;
}

static inline size_t TextStorageImpl_freeSize(const TextStorageImpl * o)
{
    //return o->textBuffer->size - o->endOfText;
    return o->textBuffer.size - o->endOfText;
}

#endif // TEXT_STORAGE_IMPL_H
