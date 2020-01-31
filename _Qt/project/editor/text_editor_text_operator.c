#include "text_editor_text_operator.h"

static bool _thereIsTextToRemove(size_t removeTextSize);
static bool _thereIsTextToWrite(const Unicode_Buf * textToWrite);

static bool _write( TextEditorTextOperator * o,
                    size_t posToWrite,
                    const Unicode_Buf * textToWrite );
static void _remove( TextEditorTextOperator * o,
                     size_t removePosition,
                     size_t removeTextSize );
static bool _removeAndWrite( TextEditorTextOperator * o,
                             size_t removeAndWritePosition,
                             size_t removeTextSize,
                             const Unicode_Buf * textToWrite );

static void _replace( TextEditorTextOperator * o,
                      size_t replacePos,
                      const Unicode_Buf * textToReplace );

static bool _replaceAndWrite( TextEditorTextOperator * o,
                              const Unicode_Buf * fullText,
                              size_t separationPos );

static void _replaceAndRemove( TextEditorTextOperator * o,
                               const Unicode_Buf * replaceText,
                               size_t fullSize );

void TextEditorTextOperator_init( TextEditorTextOperator * o,
                                  const Unicode_Buf * textBuffer )
{
    TextEditorTextStorage_init(&o->storage, textBuffer);
}

bool TextEditorTextOperator_removeAndWrite( TextEditorTextOperator * o,
                                            size_t removeAndWritePosition,
                                            size_t removeTextSize,
                                            const Unicode_Buf * textToWrite )
{
    bool thereIsTextToRemove = _thereIsTextToRemove(removeTextSize);
    bool thereIsTextToWrite  = _thereIsTextToWrite(textToWrite);

    if(thereIsTextToRemove && thereIsTextToWrite)
        return _removeAndWrite(o, removeAndWritePosition, removeTextSize, textToWrite);

    if(thereIsTextToWrite)
        return _write(o, removeAndWritePosition, textToWrite);

    if(thereIsTextToRemove)
        _remove(o, removeAndWritePosition, removeTextSize);

    return true;
}


bool _thereIsTextToRemove(size_t removeTextSize)
{
    return removeTextSize != 0;
}

bool _thereIsTextToWrite(const Unicode_Buf * textToWrite)
{
    if(textToWrite != NULL)
        if(textToWrite->size != 0)
            return true;
    return false;
}

bool _write( TextEditorTextOperator * o,
             size_t posToWrite,
             const Unicode_Buf * textToWrite )
{
    if(!_enoughSpace(o, textToWrite->size))
        return false;

    if(_posIsInFreeSpace(o, posToWrite))
        TextEditorTextStorage_append(&o->storage, textToWrite);
    else
        TextEditorTextStorage_insert(&o->storage, textToWrite, posToWrite);

    return true;
}

void _remove( TextEditorTextOperator * o,
              size_t removePosition,
              size_t removeTextSize )
{
    if(_posIsInFreeSpace(o, removePosition))
        return;

    _cutRemovingAreaIfCrossesFreeSpaceBorder( o, removePosition,
                                              &removeTextSize );

   if(_removingAreaAbutsFreeSpace(o, removePosition, removeTextSize))
       TextEditorTextStorage_truncate(&o->storage, removePosition);
   else
       TextEditorTextStorage_remove( &o->storage, removePosition,
                                     removeTextSize );
}

void _replace( TextEditorTextOperator * o,
               size_t replacePos,
               const Unicode_Buf * textToReplace )
{
    TextEditorTextStorage_replace(o, textToReplace, replacePos);
}

bool _replaceAndWrite( TextEditorTextOperator * o,
                       const Unicode_Buf * fullText,
                       size_t separationPos )
{

    buf.data = textToWrite->data;
    buf.size = removeTextSize;
    _replace(o, removeAndWritePosition, &buf);
    buf.data += removeTextSize;
    buf.size  = textToWrite->size - removeTextSize;
    return _write(o, removeAndWritePosition + removeTextSize, &buf);
}

void _replaceAndRemove( TextEditorTextOperator * o,
                        const Unicode_Buf * replaceText,
                        size_t fullSize )
{}

bool _removeAndWrite( TextEditorTextOperator * o,
                      size_t removeAndWritePosition,
                      size_t removeTextSize,
                      const Unicode_Buf * textToWrite )
{
    Unicode_Buf buf;

    if(textToWrite->size > removeTextSize)
        return _replaceAndWrite(o, textToWrite, removeTextSize);

    if(textToWrite->size == removeTextSize)
        _replace(o, removeAndWritePosition, textToWrite);
    else
        _replaceAndRemove(o, textToWrite, removeTextSize);
}

//    if(!thereIsTextToRemove && !thereIsTextToWrite)
//        ;

//    if(_posIsInFreeSpace(o, removeAndWritePosition))
//        return _checkAndAppendText(o, textToWrite);

//    if(removeTextSize == 0)
//        return _checkAndInsertText(o, textToWrite, removeAndWritePosition);

//    if(_removingAreaCrossesFreeSpace(o, removeAndWritePosition, removeTextSize))
//        _cutRemovingArea(o, removeAndWritePosition, &removeTextSize);

//    if(_)

//    if(removeTextSize < textToWrite)

//    return _removeAndWriteInsideOfText( o, removeAndWritePosition,
//                                        removeTextSize, textToWrite );
//}

bool _removeAndWriteInsideOfText( TextEditorTextOperator * o,
                                  size_t removeAndWritePosition,
                                  size_t removeTextSize,
                                  const Unicode_Buf * textToWrite )
{

    return removeTextSize == 0 ?
                _checkAndInsertText(o, textToWrite, removeAndWritePosition) :
                ;

    // Если есть текст для удаления
    else
    {
        // Если область удаления выходит за границу текста,
        //  ее нужно подрезать
        if()
            ;   // Область удаления граничит с границей текста
        else
            ;   // Проверить, граничит ли область удаления с границей текста

        // Вычислить разницу между удаляемым текстом и добавляемым


        if(/*Если текст для удаления меньше, чем для вставки*/)
        {
            if(/*Есть место*/)
            {
                ; // Заменить текст (replace)
                if(/*Граница области удаления на границе текста*/)
                    ; // Добавить текст (append)
                else
                    ; // Вставить текст (insert)
                return true;
            }
            return false;
        }

        else if(/*Если точное совпадение*/)
        {
            ; // Заменить текст (replace)
        }

        else // Если удаляемый текст больше
        {
            ; // Заменить текст (replace)
            if(/*Граница области удаления на границе текста*/)
                ; // Обрезать текст (truncate)
            else
                ; // Удалить текст (remove)
        }

        return true;
    }

}

bool _checkAndAppendText( TextEditorTextOperator * o,
                          const Unicode_Buf * text)
{
    if(_hasTextToWrite(text))
    {
        if(!_enoughSpace(o, text->size))
            return false;

        _append(o, text);
    }
    return true;
}


bool _checkAndInsertText( TextEditorTextOperator * o,
                          const Unicode_Buf * text,
                          size_t pos )
{
    if(_hasTextToWrite(text))
    {
        if(!_enoughSpace(o, text->size))
            return false;

        _insert(o, text, pos);
    }
    return true;
}






void TextEditorTextOperator_read( TextEditorTextOperator * o,
                                  size_t readPosition,
                                  Unicode_Buf * readTextBuffer )
{
    size_t endOfText = TextEditorTextStorage_endOfText(o->storage);
    if(readPosition >= endOfText)
    {
        readTextBuffer->size = 0;
        return;
    }

    size_t distToEndOfText = endOfText - readPosition;
    size_t actualReadTextSize =  distToEndOfText < readTextBuffer->size ?
                distToEndOfText : readTextBuffer->size;

    readTextBuffer->size = actualReadTextSize;
    TextEditorTextStorage_read(o->storage, readPosition, readTextBuffer);
}


bool _posIsInFreeSpace(TextEditorTextOperator * o, size_t pos)
{
    return pos >= o->endOfText;
}

bool _hasTextToWrite(const Unicode_Buf * text)
{
    if(text != NULL)
        if(text->size != 0)
            return true;
    return false;
}

bool _enoughSpace(TextEditorTextOperator * o, size_t textLen)
{
    size_t freeSize = o->textBuffer.size - o->endOfText;
    return freeSize >= textLen;
}

