#include "lpm_text_storage.h"

static void _normalizeRemovingArea( const TextBufferImpl * textStorage,
                                    LPM_SelectionCursor * removingArea );
static bool _removingAreaIsInFreeSpace( const TextBufferImpl * textStorage,
                                        const LPM_SelectionCursor * removingArea );
static void _clearRemovingAreaAndMoveToEndOfText( const TextBufferImpl * textStorage,
                                                  LPM_SelectionCursor * removingArea );
static void _cutRemovingAreaIfCrossesFreeSpaceBorder( const TextBufferImpl * textStorage,
                                                      LPM_SelectionCursor * removingArea );

static void _initTextBuffer( Unicode_Buf * localBuffer,
                             const Unicode_Buf * sourceBuffer );

static bool _notEnoughSpaceToRemoveAndWrite( const TextBufferImpl * o,
                                             const LPM_SelectionCursor * removingArea,
                                             const Unicode_Buf * textToWrite );

static void _decomposeToSimpleFxns( TextBufferImpl * textStorage,
                                    const LPM_SelectionCursor * removingArea,
                                    const Unicode_Buf * textToWrite );
static void _replaceAndWrite( TextBufferImpl * textStorage,
                              const LPM_SelectionCursor * removingArea,
                              const Unicode_Buf * textToWrite );
static void _write( TextBufferImpl * textStorage,
                    const LPM_SelectionCursor * removingArea,
                    const Unicode_Buf * textToWrite );
static void _replaceAndRemove( TextBufferImpl * textStorage,
                               const LPM_SelectionCursor * removingArea,
                               const Unicode_Buf * textToWrite );
static void _remove( TextBufferImpl * o,
                     const LPM_SelectionCursor * removingArea );
static void _replace( TextBufferImpl * textStorage,
                      const LPM_SelectionCursor * removingArea,
                      const Unicode_Buf * textToWrite );
static bool _removingAreaAbutsFreeSpace( const TextBufferImpl * textStorage,
                                         const LPM_SelectionCursor * removingArea );



bool LPM_TextStorage_replace
        ( LPM_TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite )
{
    Unicode_Buf textBuffer;

    _normalizeRemovingArea(&o->storage, removingArea);
    _initTextBuffer(&textBuffer, textToWrite);

    //printf("before ");

    if(_notEnoughSpaceToRemoveAndWrite(
                &o->storage, removingArea, &textBuffer))
    {
        //printf("pos: %d, rmlen: %d, txtlen: %d\n", removingArea->pos, removingArea->len, textBuffer.size);
        return false;
    }


    //printf("not false ");

    _decomposeToSimpleFxns(
                &o->storage, removingArea, &textBuffer);

    return true;
}

void LPM_TextStorage_read
        ( LPM_TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer )
{
    size_t endOfText = TextBufferImpl_endOfText(&o->storage);
    if(readPosition >= endOfText)
    {
        readTextBuffer->size = 0;
        return;
    }

    size_t distToEndOfText = endOfText - readPosition;
    size_t actualReadTextSize =  distToEndOfText < readTextBuffer->size ?
                distToEndOfText : readTextBuffer->size;

    readTextBuffer->size = actualReadTextSize;
    TextBufferImpl_read(&o->storage, readPosition, readTextBuffer);
}

bool LPM_TextStorage_enoughPlace
        ( LPM_TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite )
{
    Unicode_Buf textBuffer;
    _normalizeRemovingArea(&o->storage, removingArea);
    _initTextBuffer(&textBuffer, textToWrite);
    return !_notEnoughSpaceToRemoveAndWrite(&o->storage, removingArea, &textBuffer);
}

//void LPM_TextStorage_readBack
//        ( LPM_TextStorage * o,
//          size_t readPosition,
//          Unicode_Buf * readTextBuffer )
//{
//    size_t requestedSize = readTextBuffer->size;
//    size_t actualSize =
//            readPosition < requestedSize ? readPosition : requestedSize;
//    readTextBuffer->size = actualSize;
//    TextBuffer_read(&o->storage, readPosition - actualSize, readTextBuffer);
//}


void _normalizeRemovingArea( const TextBufferImpl * textStorage,
                             LPM_SelectionCursor * removingArea )
{
    if(_removingAreaIsInFreeSpace(textStorage, removingArea))
        _clearRemovingAreaAndMoveToEndOfText(textStorage, removingArea);

    _cutRemovingAreaIfCrossesFreeSpaceBorder(textStorage, removingArea);
}

bool _removingAreaIsInFreeSpace( const TextBufferImpl * textStorage,
                                 const LPM_SelectionCursor * removingArea )
{
    return removingArea->pos >= TextBufferImpl_endOfText(textStorage);
}

void _clearRemovingAreaAndMoveToEndOfText( const TextBufferImpl * textStorage,
                                           LPM_SelectionCursor * removingArea )
{
    removingArea->pos = TextBufferImpl_endOfText(textStorage);
    removingArea->len = 0;
}

void _cutRemovingAreaIfCrossesFreeSpaceBorder( const TextBufferImpl * textStorage,
                                               LPM_SelectionCursor * removingArea )
{
    size_t distToEndOfText =
            TextBufferImpl_endOfText(textStorage) - removingArea->pos;
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

bool _notEnoughSpaceToRemoveAndWrite( const TextBufferImpl * o,
                                      const LPM_SelectionCursor * removingArea,
                                      const Unicode_Buf * textToWrite )
{
    if(removingArea->len >= textToWrite->size)
        return false;

    size_t writeLen = textToWrite->size - removingArea->len;
    return writeLen > TextBufferImpl_freeSize(o);
}

void _decomposeToSimpleFxns( TextBufferImpl * textStorage,
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

void _replaceAndWrite( TextBufferImpl * textStorage,
                       const LPM_SelectionCursor * removingArea,
                       const Unicode_Buf * textToWrite )
{
    Unicode_Buf buf;
    size_t pos;

    buf.data = textToWrite->data;
    buf.size = removingArea->len;
    pos      = removingArea->pos;

    TextBufferImpl_replace(textStorage, &buf, pos);

    buf.data += removingArea->len;
    buf.size  = textToWrite->size - removingArea->len;
    pos      += removingArea->len;


    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextBufferImpl_append(textStorage, &buf);
    else
        TextBufferImpl_insert(textStorage, &buf, pos);
}

void _write( TextBufferImpl * textStorage,
             const LPM_SelectionCursor * removingArea,
             const Unicode_Buf * textToWrite )
{
    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextBufferImpl_append(textStorage, textToWrite);
    else
        TextBufferImpl_insert(textStorage, textToWrite, removingArea->pos);
}

void _replaceAndRemove( TextBufferImpl * textStorage,
                        const LPM_SelectionCursor * removingArea,
                        const Unicode_Buf * textToWrite )
{
    TextBufferImpl_replace(textStorage, textToWrite, removingArea->pos);

    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextBufferImpl_truncate(textStorage, removingArea->pos + textToWrite->size);
    else
        TextBufferImpl_remove( textStorage, removingArea->pos + textToWrite->size,
                                      removingArea->len - textToWrite->size );
}

void _remove( TextBufferImpl * textStorage,
              const LPM_SelectionCursor * removingArea )
{
    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextBufferImpl_truncate(textStorage, removingArea->pos);
    else
        TextBufferImpl_remove(textStorage, removingArea->pos, removingArea->len);
}

void _replace( TextBufferImpl * textStorage,
               const LPM_SelectionCursor * removingArea,
               const Unicode_Buf * textToWrite )
{
    TextBufferImpl_replace(textStorage, textToWrite, removingArea->pos);
}

bool _removingAreaAbutsFreeSpace( const TextBufferImpl * textStorage,
                                  const LPM_SelectionCursor * removingArea )
{
    return removingArea->pos + removingArea->len ==
            TextBufferImpl_endOfText(textStorage);
}
