#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "lpm_unicode.h"

typedef struct Clipboard
{
    Unicode_Buf * buf;
} Clipboard;

inline void Clipboard_init(Clipboard * o, Unicode_Buf * buf)
{
    o->buf = buf;
}

//void Clipboard_clear(Clipboard * obj);
//void Clipboard_write(Clipboard * obj, const Unicode_Buf * buf);
//inline const unicode_t * Clipboard_data(Clipboard * obj);
//inline size_t Clipboard_size(Clipboard * obj);

#endif // CLIPBOARD_H
