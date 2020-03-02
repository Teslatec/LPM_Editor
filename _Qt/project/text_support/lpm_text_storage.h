#ifndef LPM_TEXT_STORAGE_H
#define LPM_TEXT_STORAGE_H

#include "lpm_structs.h"
#include "text_buffer.h"

typedef struct LPM_TextStorage
{
    TextBuffer storage;
} LPM_TextStorage;

bool LPM_TextStorage_replace
        ( LPM_TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite );

void LPM_TextStorage_read
        ( LPM_TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer );

//void LPM_TextStorage_readBack
//        ( LPM_TextStorage * o,
//          size_t readPosition,
//          Unicode_Buf * readTextBuffer );

inline void LPM_TextStorage_init(LPM_TextStorage * o, Unicode_Buf * textBuffer)
{
    TextBuffer_init(&o->storage, textBuffer);
}

inline void LPM_TextStorage_sync(LPM_TextStorage * o)
{
    return TextBuffer_sync(&o->storage);
}

inline size_t LPM_TextStorage_freeSize(LPM_TextStorage * o)
{
    return TextBuffer_freeSize(&o->storage);
}

inline size_t LPM_TextStorage_endOfText(LPM_TextStorage * o)
{
    return TextBuffer_endOfText(&o->storage);
}

#endif // LPM_TEXT_STORAGE_H
