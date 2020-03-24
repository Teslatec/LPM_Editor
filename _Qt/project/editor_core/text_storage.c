#include "text_storage.h"
#include "text_buffer.h"

static void _normalizeRemovingArea( const TextStorageImpl * textStorage,
                                    LPM_SelectionCursor * removingArea );
static bool _removingAreaIsInFreeSpace( const TextStorageImpl * textStorage,
                                        const LPM_SelectionCursor * removingArea );
static void _clearRemovingAreaAndMoveToEndOfText( const TextStorageImpl * textStorage,
                                                  LPM_SelectionCursor * removingArea );
static void _cutRemovingAreaIfCrossesFreeSpaceBorder( const TextStorageImpl * textStorage,
                                                      LPM_SelectionCursor * removingArea );

static void _initTextBuffer( Unicode_Buf * localBuffer,
                             const Unicode_Buf * sourceBuffer );

static bool _notEnoughSpaceToRemoveAndWrite( const TextStorageImpl * o,
                                             const LPM_SelectionCursor * removingArea,
                                             size_t writeTextSize );

static void _decomposeToSimpleFxns( TextStorageImpl * textStorage,
                                    const LPM_SelectionCursor * removingArea,
                                    const Unicode_Buf * textToWrite );
static void _replaceAndWrite( TextStorageImpl * textStorage,
                              const LPM_SelectionCursor * removingArea,
                              const Unicode_Buf * textToWrite );
static void _write( TextStorageImpl * textStorage,
                    const LPM_SelectionCursor * removingArea,
                    const Unicode_Buf * textToWrite );
static void _replaceAndRemove( TextStorageImpl * textStorage,
                               const LPM_SelectionCursor * removingArea,
                               const Unicode_Buf * textToWrite );
static void _remove( TextStorageImpl * o,
                     const LPM_SelectionCursor * removingArea );
static void _replace( TextStorageImpl * textStorage,
                      const LPM_SelectionCursor * removingArea,
                      const Unicode_Buf * textToWrite );
static bool _removingAreaAbutsFreeSpace( const TextStorageImpl * textStorage,
                                         const LPM_SelectionCursor * removingArea );

static void _modifyRecvCursor
        ( TextStorage * o,
          const LPM_SelectionCursor * removeArea,
          const Unicode_Buf * textToWrite );

static bool _changingAreaBeyondRecvCursor( TextStorage * o,
          const LPM_SelectionCursor * removeArea,
          const Unicode_Buf * textToWrite );

static void _normalizeRecvCursor(TextStorage * o, LPM_SelectionCursor * writeCursor);

bool TextStorage_replace
        ( TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite )
{
    Unicode_Buf textBuffer;

    _normalizeRemovingArea(o->m->textStorageImpl, removingArea);
    _initTextBuffer(&textBuffer, textToWrite);

    //printf("before ");

    if(_notEnoughSpaceToRemoveAndWrite(
                o->m->textStorageImpl, removingArea, textBuffer.size))
    {
        //printf("pos: %d, rmlen: %d, txtlen: %d\n", removingArea->pos, removingArea->len, textBuffer.size);
        return false;
    }

    //printf("not false ");

    _decomposeToSimpleFxns(
                o->m->textStorageImpl, removingArea, &textBuffer);

    _modifyRecvCursor(o, removingArea, &textBuffer);

    return true;
}

void TextStorage_read
        ( TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer )
{
    size_t endOfText = TextStorageImpl_endOfText(o->m->textStorageImpl);
    if(readPosition >= endOfText)
    {
        readTextBuffer->size = 0;
        return;
    }

    size_t distToEndOfText = endOfText - readPosition;
    size_t actualReadTextSize =  distToEndOfText < readTextBuffer->size ?
                distToEndOfText : readTextBuffer->size;

    readTextBuffer->size = actualReadTextSize;
    TextStorageImpl_read(o->m->textStorageImpl, readPosition, readTextBuffer);
}

bool TextStorage_enoughPlace
        ( TextStorage * o,
          const LPM_SelectionCursor * removingArea,
          size_t writeTextSize )
{
    //Unicode_Buf textBuffer;
    LPM_SelectionCursor _remArea = { removingArea->pos, removingArea->len };
    _normalizeRemovingArea(o->m->textStorageImpl, &_remArea);
    //_initTextBuffer(&textBuffer, textToWrite);
    return !_notEnoughSpaceToRemoveAndWrite(o->m->textStorageImpl, removingArea, writeTextSize);
}

void TextStorage_sync(TextStorage * o, size_t pos)
{
    o->needToSync = false;
    o->recvCursor.pos = pos;
    o->recvCursor.len = o->m->recoveryBuffer->buffer.size;

    LPM_SelectionCursor writeCursor;
    _normalizeRecvCursor(o, &writeCursor);
    TextBuffer_push(o->m->recoveryBuffer, &writeCursor);
}

void TextStorage_recv(TextStorage * o, LPM_SelectionCursor * cursor)
{
    o->needToSync = true;

    LPM_SelectionCursor tmp = { o->recvCursor.pos, o->recvCursor.len };
    TextBuffer_pop(o->m->recoveryBuffer, &tmp);
    TextStorage_sync(o, o->recvCursor.pos);
    cursor->pos = o->recvCursor.pos;
    cursor->len = 0;
}

void _normalizeRemovingArea( const TextStorageImpl * textStorage,
                             LPM_SelectionCursor * removingArea )
{
    if(_removingAreaIsInFreeSpace(textStorage, removingArea))
        _clearRemovingAreaAndMoveToEndOfText(textStorage, removingArea);

    _cutRemovingAreaIfCrossesFreeSpaceBorder(textStorage, removingArea);
}

bool _removingAreaIsInFreeSpace( const TextStorageImpl * textStorage,
                                 const LPM_SelectionCursor * removingArea )
{
    return removingArea->pos >= TextStorageImpl_endOfText(textStorage);
}

void _clearRemovingAreaAndMoveToEndOfText( const TextStorageImpl * textStorage,
                                           LPM_SelectionCursor * removingArea )
{
    removingArea->pos = TextStorageImpl_endOfText(textStorage);
    removingArea->len = 0;
}

void _cutRemovingAreaIfCrossesFreeSpaceBorder( const TextStorageImpl * textStorage,
                                               LPM_SelectionCursor * removingArea )
{
    size_t distToEndOfText =
            TextStorageImpl_endOfText(textStorage) - removingArea->pos;
    if(distToEndOfText < removingArea->len)
        removingArea->len = distToEndOfText;
}

void _initTextBuffer( Unicode_Buf * localBuffer,
                      const Unicode_Buf * sourceBuffer )
{
    if(sourceBuffer == NULL)
    {
        localBuffer->data = NULL;
        localBuffer->size = 0;
    }
    else
    {
        localBuffer->data = sourceBuffer->data;
        localBuffer->size = sourceBuffer->size;
    }
}

bool _notEnoughSpaceToRemoveAndWrite( const TextStorageImpl * o,
                                      const LPM_SelectionCursor * removingArea,
                                      size_t writeTextSize )
{
    if(removingArea->len >= writeTextSize)
        return false;

    size_t writeLen = writeTextSize - removingArea->len;
    return writeLen > TextStorageImpl_freeSize(o);
}

void _decomposeToSimpleFxns( TextStorageImpl * textStorage,
                             const LPM_SelectionCursor * removingArea,
                             const Unicode_Buf * textToWrite )
{
    if(textToWrite->size > removingArea->len)
    {
        if(removingArea->len > 0)
            _replaceAndWrite(textStorage, removingArea, textToWrite);
        else
            _write(textStorage, removingArea, textToWrite);
    }

    else if(textToWrite->size < removingArea->len)
    {
        if(textToWrite->size > 0)
            _replaceAndRemove(textStorage, removingArea, textToWrite);
        else
            _remove(textStorage, removingArea);
    }

    else
    {
        if(textToWrite->size > 0)
            _replace(textStorage, removingArea, textToWrite);
    }
}

void _replaceAndWrite( TextStorageImpl * textStorage,
                       const LPM_SelectionCursor * removingArea,
                       const Unicode_Buf * textToWrite )
{
    Unicode_Buf buf;
    size_t pos;

    buf.data = textToWrite->data;
    buf.size = removingArea->len;
    pos      = removingArea->pos;

    TextStorageImpl_replace(textStorage, &buf, pos);

    buf.data += removingArea->len;
    buf.size  = textToWrite->size - removingArea->len;
    pos      += removingArea->len;


    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextStorageImpl_append(textStorage, &buf);
    else
        TextStorageImpl_insert(textStorage, &buf, pos);
}

void _write( TextStorageImpl * textStorage,
             const LPM_SelectionCursor * removingArea,
             const Unicode_Buf * textToWrite )
{
    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextStorageImpl_append(textStorage, textToWrite);
    else
        TextStorageImpl_insert(textStorage, textToWrite, removingArea->pos);
}

void _replaceAndRemove( TextStorageImpl * textStorage,
                        const LPM_SelectionCursor * removingArea,
                        const Unicode_Buf * textToWrite )
{
    TextStorageImpl_replace(textStorage, textToWrite, removingArea->pos);

    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextStorageImpl_truncate(textStorage, removingArea->pos + textToWrite->size);
    else
        TextStorageImpl_remove( textStorage, removingArea->pos + textToWrite->size,
                                      removingArea->len - textToWrite->size );
}

void _remove( TextStorageImpl * textStorage,
              const LPM_SelectionCursor * removingArea )
{
    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextStorageImpl_truncate(textStorage, removingArea->pos);
    else
        TextStorageImpl_remove(textStorage, removingArea->pos, removingArea->len);
}

void _replace( TextStorageImpl * textStorage,
               const LPM_SelectionCursor * removingArea,
               const Unicode_Buf * textToWrite )
{
    TextStorageImpl_replace(textStorage, textToWrite, removingArea->pos);
}

bool _removingAreaAbutsFreeSpace( const TextStorageImpl * textStorage,
                                  const LPM_SelectionCursor * removingArea )
{
    return removingArea->pos + removingArea->len ==
            TextStorageImpl_endOfText(textStorage);
}

void _modifyRecvCursor
        ( TextStorage * o,
          const LPM_SelectionCursor * removeArea,
          const Unicode_Buf * textToWrite )
{
    if(o->needToSync)
        return;

    if(_changingAreaBeyondRecvCursor(o, removeArea, textToWrite))
    {
        o->needToSync = true;
        return;
    }

    // Замена
    if(removeArea->len == textToWrite->size)
        return;

    // Вставка
    if(removeArea->len < textToWrite->size)
    {
        o->recvCursor.len += textToWrite->size - removeArea->len;
        return;
    }

    // Удаление
    size_t remLen = removeArea->len - textToWrite->size;
    if(remLen > o->recvCursor.len)
    {
        o->needToSync = true;
        return;
    }

    o->recvCursor.len -= remLen;
}

bool _changingAreaBeyondRecvCursor
        ( TextStorage * o,
          const LPM_SelectionCursor * removeArea,
          const Unicode_Buf * textToWrite )
{
    size_t len = removeArea->len > textToWrite->size ?
                 removeArea->len : textToWrite->size;

//    test_print_text_cursor(removeArea->pos, len);
//    test_print_text_cursor(o->recvCursor.pos, o->recvCursor.len);

    if(removeArea->pos < o->recvCursor.pos)
        return true;

    if(removeArea->pos + len > o->recvCursor.pos + o->recvCursor.len)
        return true;

    return false;
}

void _normalizeRecvCursor(TextStorage * o, LPM_SelectionCursor * writeCursor)
{
    writeCursor->pos = o->recvCursor.pos;
    writeCursor->len = o->recvCursor.len;

    size_t endOfText = TextStorageImpl_endOfText(o->m->textStorageImpl);
    size_t endOfCurs = o->recvCursor.pos + o->recvCursor.len;
    if(endOfText < endOfCurs)
        writeCursor->len -= endOfCurs - endOfText;
}
