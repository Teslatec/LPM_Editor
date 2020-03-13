#ifndef TEXT_STORAGE_H
#define TEXT_STORAGE_H

#include "lpm_structs.h"
#include "text_storage_impl.h"

typedef struct TextStorage
{
    TextStorageImpl * storage;
} TextStorage;

bool TextStorage_replace
        ( TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite );

void TextStorage_read
        ( TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer );

bool TextStorage_enoughPlace
        ( TextStorage * o,
          const LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite );

static inline void TextStorage_init
        ( TextStorage * o,
          TextStorageImpl * storageImpl)
{
    o->storage = storageImpl;
}

static inline void TextStorage_clear(TextStorage * o, bool deep)
{
    TextStorageImpl_clear(o->storage, deep);
}

static inline void TextStorage_sync(TextStorage * o)
{
    return TextStorageImpl_sync(o->storage);
}

static inline size_t TextStorage_freeSize(TextStorage * o)
{
    return TextStorageImpl_freeSize(o->storage);
}

static inline size_t TextStorage_endOfText(TextStorage * o)
{
    return TextStorageImpl_endOfText(o->storage);
}

#endif // TEXT_STORAGE_H
