#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "lpm_unicode.h"
#include "lpm_structs.h"
#include "modules.h"

typedef struct Clipboard
{
    const Modules * modules;
    size_t currLen;
} Clipboard;

static inline void Clipboard_init
    ( Clipboard * o,
      const Modules * modules)
{
    o->modules = modules;
    o->currLen = 0;
}

static inline void Clipboard_clear(Clipboard * o) { o->currLen = 0; }

bool Clipboard_push
        ( Clipboard * o,
          const LPM_SelectionCursor * text );

bool Clipboard_pop
        ( Clipboard * o,
          LPM_SelectionCursor * text );

#endif // CLIPBOARD_H
