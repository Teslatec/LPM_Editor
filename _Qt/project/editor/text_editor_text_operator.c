#include "text_editor_text_operator.h"
#include <string.h>

static size_t _calcEndOfTextPosition(TextEditorTextOperator * o);

void TextEditorTextOperator_init( TextEditorTextOperator * o,
                                  const Unicode_Buf * textBuffer )
{
    o->textBuffer.data = textBuffer->data;
    o->textBuffer.size = textBuffer->size;
    o->endOfText = _calcEndOfTextPosition(o);
}

bool TextEditorTextOperator_removeAndWrite( TextEditorTextOperator * o,
                                            size_t removeAndWritePosition,
                                            size_t removeTextSize,
                                            const Unicode_Buf * textToWrite )
{
    // Позиция ввода на или за границей текста
    if(removeAndWritePosition >= o->endOfText)
    {
        if(/*Если есть текст для записи*/)
        {
            if(/*Есть место*/)
            {
                ; // Добавить в конец (append)
                return true;
            }
            return false;
        }
        return true;
    }

    // Позиция ввода в области текста
    else
    {
        // Если размер удаляемого текста - ноль
        if(removeTextSize == 0)
        {
            if(/*Если есть текст для записи*/)
            {
                if(/*Есть место*/)
                {
                    ; // Вставить текст (insert)
                    return true;
                }
                return false;
            }
            return true;
        }

        // Если есть текст для удаления
        else
        {
            // Если область удаления выходит за границу текста,
            //  ее нужно подрезать
            if()
                ;

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
}


void TextEditorTextOperator_read( TextEditorTextOperator * o,
                                  size_t readPosition,
                                  Unicode_Buf * readTextBuffer )
{
    if(readPosition >= o->endOfText)
    {
        readTextBuffer->size = 0;
        return;
    }

    size_t distToEndOfText = o->endOfText - readPosition;
    size_t actualReadTextSize =  distToEndOfText < readTextBuffer->size ?
                distToEndOfText : readTextBuffer->size;

    memcpy( readTextBuffer->data,
            o->textBuffer.data + readPosition,
            actualReadTextSize * sizeof(unicode_t) );
    readTextBuffer->size = actualReadTextSize;
}

size_t _calcEndOfTextPosition(TextEditorTextOperator * o)
{
    const unicode_t * begin = o->textBuffer.data;
    const unicode_t * end   = o->textBuffer.data + o->textBuffer.size;
    const unicode_t * ptr;
    for(ptr = begin; ptr != end; ptr++)
        if(*ptr == 0x0000)
            break;
    return ptr - begin;
}
