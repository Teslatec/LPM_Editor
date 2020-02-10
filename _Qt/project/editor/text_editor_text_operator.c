#include "text_editor_text_operator.h"

static void _normalizeRemovingArea( const TextEditorTextStorage * textStorage,
                                    LPM_SelectionCursor * removingArea );
static bool _removingAreaIsInFreeSpace( const TextEditorTextStorage * textStorage,
                                        const LPM_SelectionCursor * removingArea );
static void _clearRemovingAreaAndMoveToEndOfText( const TextEditorTextStorage * textStorage,
                                                  LPM_SelectionCursor * removingArea );
static void _cutRemovingAreaIfCrossesFreeSpaceBorder( const TextEditorTextStorage * textStorage,
                                                      LPM_SelectionCursor * removingArea );

static void _initTextBuffer( Unicode_Buf * localBuffer,
                             const Unicode_Buf * sourceBuffer );

static bool _notEnoughSpaceToRemoveAndWrite( const TextEditorTextStorage * o,
                                             const LPM_SelectionCursor * removingArea,
                                             const Unicode_Buf * textToWrite );

static void _decomposeToSimpleFxns( TextEditorTextStorage * textStorage,
                                    const LPM_SelectionCursor * removingArea,
                                    const Unicode_Buf * textToWrite );
static void _replaceAndWrite( TextEditorTextStorage * textStorage,
                              const LPM_SelectionCursor * removingArea,
                              const Unicode_Buf * textToWrite );
static void _write( TextEditorTextStorage * textStorage,
                    const LPM_SelectionCursor * removingArea,
                    const Unicode_Buf * textToWrite );
static void _replaceAndRemove( TextEditorTextStorage * textStorage,
                               const LPM_SelectionCursor * removingArea,
                               const Unicode_Buf * textToWrite );
static void _remove( TextEditorTextStorage * o,
                     const LPM_SelectionCursor * removingArea );
static void _replace( TextEditorTextStorage * textStorage,
                      const LPM_SelectionCursor * removingArea,
                      const Unicode_Buf * textToWrite );
static bool _removingAreaAbutsFreeSpace( const TextEditorTextStorage * textStorage,
                                         const LPM_SelectionCursor * removingArea );



bool TextEditorTextOperator_removeAndWrite( TextEditorTextOperator * o,
                                            LPM_SelectionCursor * removingArea,
                                            const Unicode_Buf * textToWrite )
{
    Unicode_Buf textBuffer;

    _normalizeRemovingArea(o->textStorage, removingArea);
    _initTextBuffer(&textBuffer, textToWrite);

    //printf("before ");

    if(_notEnoughSpaceToRemoveAndWrite(
                o->textStorage, removingArea, &textBuffer))
    {
        //printf("pos: %d, rmlen: %d, txtlen: %d\n", removingArea->pos, removingArea->len, textBuffer.size);
        return false;
    }


    //printf("not false ");

    _decomposeToSimpleFxns(
                o->textStorage, removingArea, &textBuffer);

    return true;
}

void TextEditorTextOperator_read( TextEditorTextOperator * o,
                                  size_t readPosition,
                                  Unicode_Buf * readTextBuffer )
{
    size_t endOfText = TextEditorTextStorage_endOfText(o->textStorage);
    if(readPosition >= endOfText)
    {
        readTextBuffer->size = 0;
        return;
    }

    size_t distToEndOfText = endOfText - readPosition;
    size_t actualReadTextSize =  distToEndOfText < readTextBuffer->size ?
                distToEndOfText : readTextBuffer->size;

    readTextBuffer->size = actualReadTextSize;
    TextEditorTextStorage_read(o->textStorage, readPosition, readTextBuffer);
}



void _normalizeRemovingArea( const TextEditorTextStorage * textStorage,
                             LPM_SelectionCursor * removingArea )
{
    if(_removingAreaIsInFreeSpace(textStorage, removingArea))
        _clearRemovingAreaAndMoveToEndOfText(textStorage, removingArea);

    _cutRemovingAreaIfCrossesFreeSpaceBorder(textStorage, removingArea);
}

bool _removingAreaIsInFreeSpace( const TextEditorTextStorage * textStorage,
                                 const LPM_SelectionCursor * removingArea )
{
    return removingArea->pos >= TextEditorTextStorage_endOfText(textStorage);
}

void _clearRemovingAreaAndMoveToEndOfText( const TextEditorTextStorage * textStorage,
                                           LPM_SelectionCursor * removingArea )
{
    removingArea->pos = TextEditorTextStorage_endOfText(textStorage);
    removingArea->len = 0;
}

void _cutRemovingAreaIfCrossesFreeSpaceBorder( const TextEditorTextStorage * textStorage,
                                               LPM_SelectionCursor * removingArea )
{
    size_t distToEndOfText =
            TextEditorTextStorage_endOfText(textStorage) - removingArea->pos;
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

bool _notEnoughSpaceToRemoveAndWrite( const TextEditorTextStorage * o,
                                      const LPM_SelectionCursor * removingArea,
                                      const Unicode_Buf * textToWrite )
{
    if(removingArea->len >= textToWrite->size)
        return false;

    //printf("Aaaaa!!! ");

    size_t writeLen = textToWrite->size - removingArea->len;
    return writeLen > TextEditorTextStorage_freeSize(o);
}

void _decomposeToSimpleFxns( TextEditorTextStorage * textStorage,
                             const LPM_SelectionCursor * removingArea,
                             const Unicode_Buf * textToWrite )
{
    //printf("wsz: %d rsz: %d ", textToWrite->size, removingArea->len);
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

void _replaceAndWrite( TextEditorTextStorage * textStorage,
                       const LPM_SelectionCursor * removingArea,
                       const Unicode_Buf * textToWrite )
{
    Unicode_Buf buf;
    size_t pos;

    buf.data = textToWrite->data;
    buf.size = removingArea->len;
    pos      = removingArea->pos;

    TextEditorTextStorage_replace(textStorage, &buf, pos);

    buf.data += removingArea->len;
    buf.size  = textToWrite->size - removingArea->len;
    pos      += removingArea->len;


    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextEditorTextStorage_append(textStorage, &buf);
    else
        TextEditorTextStorage_insert(textStorage, &buf, pos);
}

void _write( TextEditorTextStorage * textStorage,
             const LPM_SelectionCursor * removingArea,
             const Unicode_Buf * textToWrite )
{
    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextEditorTextStorage_append(textStorage, textToWrite);
    else
        TextEditorTextStorage_insert(textStorage, textToWrite, removingArea->pos);
}

void _replaceAndRemove( TextEditorTextStorage * textStorage,
                        const LPM_SelectionCursor * removingArea,
                        const Unicode_Buf * textToWrite )
{
    TextEditorTextStorage_replace(textStorage, textToWrite, removingArea->pos);

    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextEditorTextStorage_truncate(textStorage, removingArea->pos + textToWrite->size);
    else
        TextEditorTextStorage_remove( textStorage, removingArea->pos + textToWrite->size,
                                      removingArea->len - textToWrite->size );
}

void _remove( TextEditorTextStorage * textStorage,
              const LPM_SelectionCursor * removingArea )
{
    if(_removingAreaAbutsFreeSpace(textStorage, removingArea))
        TextEditorTextStorage_truncate(textStorage, removingArea->pos);
    else
        TextEditorTextStorage_remove(textStorage, removingArea->pos, removingArea->len);
}

void _replace( TextEditorTextStorage * textStorage,
               const LPM_SelectionCursor * removingArea,
               const Unicode_Buf * textToWrite )
{
    TextEditorTextStorage_replace(textStorage, textToWrite, removingArea->pos);
}

bool _removingAreaAbutsFreeSpace( const TextEditorTextStorage * textStorage,
                                  const LPM_SelectionCursor * removingArea )
{
    return removingArea->pos + removingArea->len ==
            TextEditorTextStorage_endOfText(textStorage);
}
