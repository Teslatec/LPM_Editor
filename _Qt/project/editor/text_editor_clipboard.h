#ifndef TEXT_EDITOR_CLIPBOARD_H
#define TEXT_EDITOR_CLIPBOARD_H

#include "lpm_unicode.h"

typedef struct TextEditorClipboard
{
    Unicode_Buf * buf;
} TextEditorClipboard;

inline void TextEditorClipboard_init(TextEditorClipboard * o, Unicode_Buf * buf)
{
    o->buf = buf;
}

void TextEditorClipboard_clear(TextEditorClipboard * obj);
void TextEditorClipboard_write( TextEditorClipboard * obj,
                                const Unicode_Buf * buf);

inline const unicode_t * TextEditorClipboard_data(TextEditorClipboard * obj);
inline size_t TextEditorClipboard_size(TextEditorClipboard * obj);

#endif // TEXT_EDITOR_CLIPBOARD_H
