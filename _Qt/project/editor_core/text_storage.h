#ifndef TEXT_STORAGE_H
#define TEXT_STORAGE_H

#include "lpm_structs.h"
#include "text_storage_impl.h"
#include "modules.h"

typedef struct TextStorage
{
    const Modules * m;
    LPM_SelectionCursor recvCursor;
    bool needToSync;
} TextStorage;

bool TextStorage_replace
        ( TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite );

void TextStorage_read
        ( TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer );

//bool TextStorage_enoughPlace
//        ( TextStorage * o,
//          const LPM_SelectionCursor * removingArea,
//          const Unicode_Buf * textToWrite );

bool TextStorage_enoughPlace
        ( TextStorage * o,
          const LPM_SelectionCursor * removingArea,
          size_t writeTextSize );

void TextStorage_sync(TextStorage * o, size_t pos);
void TextStorage_recv(TextStorage * o, LPM_SelectionCursor * cursor);

static inline void TextStorage_init
        ( TextStorage * o,
          const Modules * m )
{
    o->m = m;
    o->needToSync = false;
}

static inline void TextStorage_clear(TextStorage * o, bool deep)
{
    TextStorageImpl_clear(o->m->textStorageImpl, deep);
}

static inline size_t TextStorage_freeSize(TextStorage * o)
{
    return TextStorageImpl_freeSize(o->m->textStorageImpl);
}

static inline size_t TextStorage_endOfText(TextStorage * o)
{
    return TextStorageImpl_endOfText(o->m->textStorageImpl);
}

static inline bool TextStorage_needToSync(TextStorage * o)
{
    return o->needToSync;
}

#endif // TEXT_STORAGE_H
