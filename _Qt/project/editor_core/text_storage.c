#include "text_storage.h"

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
                                             const Unicode_Buf * textToWrite );

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



bool TextStorage_replace
        ( TextStorage * o,
          LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite )
{
    Unicode_Buf textBuffer;

    _normalizeRemovingArea(o->storage, removingArea);
    _initTextBuffer(&textBuffer, textToWrite);

    //printf("before ");

    if(_notEnoughSpaceToRemoveAndWrite(
                o->storage, removingArea, &textBuffer))
    {
        //printf("pos: %d, rmlen: %d, txtlen: %d\n", removingArea->pos, removingArea->len, textBuffer.size);
        return false;
    }


    //printf("not false ");

    _decomposeToSimpleFxns(
                o->storage, removingArea, &textBuffer);

    return true;
}

void TextStorage_read
        ( TextStorage * o,
          size_t readPosition,
          Unicode_Buf * readTextBuffer )
{
    size_t endOfText = TextStorageImpl_endOfText(o->storage);
    if(readPosition >= endOfText)
    {
        readTextBuffer->size = 0;
        return;
    }

    size_t distToEndOfText = endOfText - readPosition;
    size_t actualReadTextSize =  distToEndOfText < readTextBuffer->size ?
                distToEndOfText : readTextBuffer->size;

    readTextBuffer->size = actualReadTextSize;
    TextStorageImpl_read(o->storage, readPosition, readTextBuffer);
}

bool TextStorage_enoughPlace
        ( TextStorage * o,
          const LPM_SelectionCursor * removingArea,
          const Unicode_Buf * textToWrite )
{
    Unicode_Buf textBuffer;
    LPM_SelectionCursor _remArea = { removingArea->pos, removingArea->len };
    _normalizeRemovingArea(o->storage, &_remArea);
    _initTextBuffer(&textBuffer, textToWrite);
    return !_notEnoughSpaceToRemoveAndWrite(o->storage, removingArea, &textBuffer);
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
                                      const Unicode_Buf * textToWrite )
{
    if(removingArea->len >= textToWrite->size)
        return false;

    size_t writeLen = textToWrite->size - removingArea->len;
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
