#ifndef LPM_TEXT_BUFFER_H
#define LPM_TEXT_BUFFER_H

#include "modules.h"

typedef struct LPM_TextBuffer
{
    Unicode_Buf buffer;
    size_t usedSize;
    Modules * modules;
} LPM_TextBuffer;


void LPM_TextBuffer_init
    (LPM_TextBuffer * o,
      const Unicode_Buf * buffer,
      Modules * modules );

void LPM_TextBuffer_clear(LPM_TextBuffer * o);

bool LPM_TextBuffer_push
    ( LPM_TextBuffer * o,
      const LPM_SelectionCursor * textCursor );

bool LPM_TextBuffer_pop
    ( LPM_TextBuffer * o,
      LPM_SelectionCursor * textCursor );

bool LPM_TextBuffer_checkPlaceInTextStorage
    ( LPM_TextBuffer * o,
      const LPM_SelectionCursor * textCursor );

static inline bool LPM_TextBuffer_isEmpty(LPM_TextBuffer * o)
{
    return o->usedSize == 0;
}

#endif // LPM_TEXT_BUFFER_H
