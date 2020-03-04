#ifndef LPM_TEXT_STORAGE_H
#define LPM_TEXT_STORAGE_H

#include "lpm_structs.h"
#include "text_storage_impl.h"

typedef struct LPM_TextStorage
{
    TextStorageImpl storage;
} LPM_TextStorage;

bool LPM_TextStorage_replace
        ( LPM_TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite );

void LPM_TextStorage_read
        ( LPM_TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer );

bool LPM_TextStorage_enoughPlace
        ( LPM_TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite );

static inline void LPM_TextStorage_init
        ( LPM_TextStorage * o,
          Unicode_Buf * textBuffer )
{
    TextStorageImpl_init(&o->storage, textBuffer);
}

static inline void LPM_TextStorage_sync(LPM_TextStorage * o)
{
    return TextStorageImpl_sync(&o->storage);
}

static inline size_t LPM_TextStorage_freeSize(LPM_TextStorage * o)
{
    return TextStorageImpl_freeSize(&o->storage);
}

static inline size_t LPM_TextStorage_endOfText(LPM_TextStorage * o)
{
    return TextStorageImpl_endOfText(&o->storage);
}

#endif // LPM_TEXT_STORAGE_H
