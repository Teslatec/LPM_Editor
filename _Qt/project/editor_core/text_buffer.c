#include "text_buffer.h"
#include "text_storage.h"
#include "line_buffer_support.h"
#include "page_formatter.h"

/*
 * Примечание: чтобы оптимизировать работу с внешней памятью в микроконтроллере
 *  нужно переписать функции _read и _write
 */

static bool _noEnoughPlaceInBuffer(TextBuffer * o, size_t len);
static bool _noEnoughPlaceInTextStorage(TextBuffer * o, const LPM_SelectionCursor * removingArea);
static void _write(TextBuffer * o, size_t pos, size_t len);
static void _read(TextBuffer * o, size_t pos, size_t len);
static size_t _calcPartialRemoveLen(size_t enterTextPos, size_t removeEndPos, size_t loadSize);

void TextBuffer_init
    ( TextBuffer * o,
      const Unicode_Buf * buffer,
      const Modules * modules )
{
    o->buffer.data = buffer->data;
    o->buffer.size = buffer->size;
    o->usedSize    = 0;
    o->modules     = modules;
}

void TextBuffer_clear(TextBuffer * o)
{
    o->usedSize = 0;
}

bool TextBuffer_push
    ( TextBuffer * o,
      const LPM_SelectionCursor * textCursor )
{
    if(_noEnoughPlaceInBuffer(o, textCursor->len))
        return false;

    o->usedSize = textCursor->len;

    const size_t partSize = o->modules->copyBuffer.size;
    size_t restSize = textCursor->len;
    size_t txtPos = textCursor->pos;
    size_t bufPos = 0;
    for(;;)
    {
        bool lastPieceReatched = partSize >= restSize;
        size_t loadSize = lastPieceReatched ? restSize : partSize;

        //LineBuffer_LoadText(o->modules, txtPos, loadSize);
        Unicode_Buf buf = { o->modules->copyBuffer.data, loadSize };
        TextStorage_read(o->modules->textStorage, txtPos, &buf);

        _write(o, bufPos, loadSize);

        if(lastPieceReatched)
            break;

        bufPos += partSize;
        txtPos += partSize;
        restSize -= partSize;
    }

    return true;
}

bool TextBuffer_pop
    ( TextBuffer * o,
      LPM_SelectionCursor * textCursor )
{
    if(o->usedSize == 0)
        return true;

    //size_t addCharsAmount = insertAddChars ? PageFormatter_addChars(o->modules->pageFormatter) : 0;

    if(_noEnoughPlaceInTextStorage(o, textCursor))
        return false;

    const size_t partSize     = o->modules->copyBuffer.size;
    const size_t removeEndPos = textCursor->pos + textCursor->len;
    size_t enterTextPos = textCursor->pos;
    size_t clipboardPos = 0;
    size_t restSize     = o->usedSize;

    Unicode_Buf partEnterText;
    LPM_SelectionCursor partRemoveArea;

    for(;;)
    {
        bool lastPieceReatched  = partSize >= restSize;
        size_t loadSize = lastPieceReatched ? restSize : partSize;

        partEnterText.data = o->modules->copyBuffer.data;
        partEnterText.size = loadSize;
        partRemoveArea.pos = enterTextPos;
        partRemoveArea.len = _calcPartialRemoveLen(enterTextPos, removeEndPos, loadSize);

        _read(o, clipboardPos, loadSize);
        TextStorage_replace(o->modules->textStorage, &partRemoveArea, &partEnterText);

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
        TextStorage_replace(o->modules->textStorage, &partRemoveArea, NULL);
    }

    textCursor->pos = enterTextPos;
    textCursor->len = 0;

    return true;
}

bool _noEnoughPlaceInBuffer(TextBuffer * o, size_t len)
{
    return len > o->buffer.size;
}

bool _noEnoughPlaceInTextStorage(TextBuffer * o, const LPM_SelectionCursor * removingArea)
{
    //Unicode_Buf textToWrite = { o->buffer.data, o->usedSize };
    return !TextStorage_enoughPlace( o->modules->textStorage,
                                     removingArea,
                                     o->usedSize );
}

void _write(TextBuffer * o, size_t pos, size_t len)
{
    memcpy( o->buffer.data + pos,
            o->modules->copyBuffer.data,
            len * sizeof(unicode_t) );
}

void _read(TextBuffer * o, size_t pos, size_t len)
{
    memcpy( o->modules->copyBuffer.data,
            o->buffer.data + pos,
            len * sizeof(unicode_t) );
}

size_t _calcPartialRemoveLen(size_t enterTextPos, size_t removeEndPos, size_t loadSize)
{
    if(enterTextPos >= removeEndPos)
        return 0;

    size_t fullRemoveLen = removeEndPos - enterTextPos;
    return fullRemoveLen >= loadSize ? loadSize : fullRemoveLen;
}
