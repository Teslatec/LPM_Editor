#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include "modules.h"

typedef struct TextBuffer
{
    Unicode_Buf buffer;
    size_t usedSize;
    const Modules * modules;
} TextBuffer;


void TextBuffer_init
    ( TextBuffer * o,
      const Unicode_Buf * buffer,
      const Modules * modules );

void TextBuffer_clear(TextBuffer * o);

bool TextBuffer_push
    ( TextBuffer * o,
      const LPM_SelectionCursor * textCursor );

bool TextBuffer_pop
    ( TextBuffer * o,
      LPM_SelectionCursor * textCursor );

static inline bool TextBuffer_isEmpty(TextBuffer * o)
{
    return o->usedSize == 0;
}

#endif // TEXT_BUFFER_H
