#include "clipboard.h"
#include "lpm_text_storage.h"
#include "line_buffer_support.h"

static bool _noEnoughPlaceInClipboard(Clipboard * o, size_t len);
static bool _noEnoughPlaceInTextStorage(Clipboard * o, const LPM_SelectionCursor * removingArea);
static void _write(Clipboard * o, size_t pos, size_t len);
static void _read(Clipboard * o, size_t pos, size_t len);
static size_t _calcPartialRemoveLen(size_t enterTextPos, size_t removeEndPos, size_t loadSize);

bool Clipboard_push
        ( Clipboard * o,
          const LPM_SelectionCursor * text )
{
    if(_noEnoughPlaceInClipboard(o, text->len))
        return false;

    o->currLen = text->len;

    const size_t partSize = o->modules->lineBuffer.size;
    size_t restSize = text->len;
    size_t txtPos = text->pos;
    size_t bufPos = 0;
    for(;;)
    {
        bool lastPieceReatched = partSize >= restSize;
        size_t loadSize = lastPieceReatched ? restSize : partSize;

        LineBuffer_LoadText(o->modules, txtPos, loadSize);
        _write(o, bufPos, loadSize);

        if(lastPieceReatched)
            break;

        bufPos += partSize;
        txtPos += partSize;
        restSize -= partSize;
    }

    return true;
}

bool Clipboard_pop
        ( Clipboard * o,
          LPM_SelectionCursor * text )
{
    if(o->currLen == 0)
        return true;

    if(_noEnoughPlaceInTextStorage(o, text))
        return false;

    const size_t partSize     = o->modules->lineBuffer.size;
    const size_t removeEndPos = text->pos + text->len;
    size_t enterTextPos = text->pos;
    size_t clipboardPos = 0;
    size_t restSize     = o->currLen;

    Unicode_Buf partEnterText;
    LPM_SelectionCursor partRemoveArea;

    for(;;)
    {
        bool lastPieceReatched  = partSize >= restSize;
        size_t loadSize = lastPieceReatched ? restSize : partSize;

        partEnterText.data = o->modules->lineBuffer.data;
        partEnterText.size = loadSize;
        partRemoveArea.pos = enterTextPos;
        partRemoveArea.len = _calcPartialRemoveLen(enterTextPos, removeEndPos, loadSize);

        _read(o, clipboardPos, loadSize);
        LPM_TextStorage_replace(o->modules->textStorage, &partRemoveArea, &partEnterText);

        if(lastPieceReatched)
        {
            enterTextPos += loadSize;
            break;
        }

        enterTextPos += partSize;
        clipboardPos += partSize;
        restSize -= partSize;
    }

    if(enterTextPos < removeEndPos)
    {
        partRemoveArea.pos = enterTextPos;
        partRemoveArea.len = removeEndPos - enterTextPos;
        LPM_TextStorage_replace(o->modules->textStorage, &partRemoveArea, NULL);
    }

    text->pos = enterTextPos;
    text->len = 0;

    return true;
}

bool _noEnoughPlaceInClipboard(Clipboard * o, size_t len)
{
    return len > o->modules->clipboardBuffer.size;
}

bool _noEnoughPlaceInTextStorage(Clipboard * o, const LPM_SelectionCursor * removingArea)
{
    if(removingArea->len < o->currLen)
        if((o->currLen - removingArea->len) >
                LPM_TextStorage_freeSize(o->modules->textStorage))
            return true;
    return false;
}

void _write(Clipboard * o, size_t pos, size_t len)
{
    memcpy( o->modules->clipboardBuffer.data + pos,
            o->modules->lineBuffer.data,
            len * sizeof(unicode_t) );
}

void _read(Clipboard * o, size_t pos, size_t len)
{
    memcpy( o->modules->lineBuffer.data,
            o->modules->clipboardBuffer.data + pos,
            len * sizeof(unicode_t) );
}

size_t _calcPartialRemoveLen(size_t enterTextPos, size_t removeEndPos, size_t loadSize)
{
    if(enterTextPos >= removeEndPos)
        return 0;

    size_t fullRemoveLen = removeEndPos - enterTextPos;
    return fullRemoveLen >= loadSize ? loadSize : fullRemoveLen;
}
