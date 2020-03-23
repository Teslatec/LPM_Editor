#ifndef LINE_BUFFER_SUPPORT_H
#define LINE_BUFFER_SUPPORT_H

#include "text_storage.h"
#include "modules.h"
#include <string.h>

static inline unicode_t * LineBuffer_LoadText(const Modules * m, size_t pos, size_t amount)
{
    // После выполнения функции LPM_TextStorage_read параметр buf.size может
    //  стать меньше amount, если при чтении достигли конца текста. В этом
    //  случае остаток буфера строки добивается нулями

    Unicode_Buf buf = { m->lineBuffer.data, amount };
    TextStorage_read(m->textStorage, pos, &buf);
    memset(buf.data + buf.size, 0, (amount - buf.size)*sizeof(unicode_t));
    return m->lineBuffer.data;
}

static inline unicode_t * LineBuffer_LoadTextBack(const Modules * m, size_t pos, size_t amount)
{
    size_t actualAmount = pos < amount ? pos : amount;
    size_t restAmount = amount - actualAmount;
    memset(m->lineBuffer.data, 0, restAmount*sizeof(unicode_t));
    Unicode_Buf buf = { m->lineBuffer.data+restAmount, actualAmount };
    TextStorage_read(m->textStorage, pos-actualAmount, &buf);
    return buf.data + actualAmount;
}

static inline unicode_t * Buffer_LoadTextBack
        ( TextStorage * storage,
          size_t pos,
          Unicode_Buf * buf )
{
    buf->size = pos < buf->size ? pos : buf->size;
    TextStorage_read(storage, pos - buf->size, buf);
    return buf->data + buf->size;
//    size_t actualAmount = pos < buf->size ? pos : buf->size;
//    size_t restAmount = buf->size - actualAmount;
//    memset(buf->data, 0, restAmount*sizeof(unicode_t));
//    Unicode_Buf tmp = { buf->data+restAmount, actualAmount };
//    TextStorage_read(storage, pos-actualAmount, &tmp);
//    return tmp.data;
}

#endif // LINE_BUFFER_SUPPORT_H
