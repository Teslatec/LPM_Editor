#include "core.h"
#include "command_reader.h"
#include "text_storage.h"
#include "page_formatter.h"
#include "text_operator.h"
#include "text_buffer.h"
#include "line_buffer_support.h"
#include "screen_painter.h"

#include <string.h>
#include <stdio.h>

#define FLAG_HAS_ACTION_TO_UNDO (0x01)
#define FLAG_READ_ONLY          (0x02)
#define FLAG_TEMPLATE_MODE      (0x04)
#define FLAG_INSERTIONS_MODE    (0x08)

typedef Core Obj;
typedef LPM_SelectionCursor SlcCurs;
typedef void(*CmdHandler)(Core*);

static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

void test_print_display_cursor(size_t bx, size_t by, size_t ex, size_t ey);
void test_print_text_cursor(size_t pos, size_t len);
void test_print_page_map(size_t base, const LineMap * prev, const LineMap * table);
void test_print_unicode(const unicode_t * buf, size_t size);
void test_beep();

static void _prepare(Obj * o);

static void _cursorChangedCmdHandler(Core * o);
static void _textChangedCmdHandler(Core * o);
static void _changeModeCmdHandler(Core * o);
static void _copyCmdHandler(Core * o);
static void _pastCmdHandler(Core * o);
static void _cutCmdHandler(Core * o);
static void _saveCmdHandler(Core * o);
static void _clearClipboardCmdHandler(Core * o);
static void _undoHandler(Core * o);
static void _recvHandler(Core * o);
static void _outlineHelpCmdHandler(Core * o);
static void _outlineStateHandler(Core * o);
static void _timeoutCmdHandler(Core * o);

static bool _processEnteredChar(Core * o);
static bool _processEnteredTab(Core * o);
static bool _processEnteredNewLine(Core * o);
static bool _processEnteredInsertionBorder(Core * o);
static void _processRemoveNextChar(Core * o);
static void _processRemovePrevChar(Core * o);
static bool _processRemovePage(Core * o);
static bool _processTruncateLine(Core * o);

static void _saveAction(Core * o, size_t insertTextSize);
static void _undoAction(Core * o);

static void _handleNotEnoughPlaceInTextStorage(Core * o);
static void _handleNotEnoughPlaceInClipboard(Core * o);

static bool _checkCharsAndPrepareToWrite(Core * o, Unicode_Buf * text, bool * pureDiacritic);
static size_t _insertPrevCharsToLineBuffer(Core * o);
static size_t _insertAddCharsToLineBuffer(Core * o, size_t offset);
static size_t _insertInputCharsToLineBuffer(Core * o, size_t offset, bool * pureDiacritic);

static bool _checkPastSizeWriteAddCharsAndSaveAction(Core * o);

static void _insertTabSeqToLineBuffer(Core * o, size_t offset);
static void _setTextBufToEndlSeq(Core * o, Unicode_Buf * text);

static bool _enterTextDespiteInsertionMode(Core * o, const Unicode_Buf * text);

static bool _readOnlyMode(Core * o);
static bool _canEnterInsertionBorderChar(Core * o);
static void _setHasActionToUndo(Core * o, bool state);
static bool _hasNotActionToUndo(Core * o);

static void _syncTextStorage(Core * o);

void _printLineMap(Core * o);
void _printDisplayCursor(Core * o);
void _printTextCursor(Core * o);

static const CmdHandler cmdHandlerTable[] =
{
    &_cursorChangedCmdHandler,
    &_textChangedCmdHandler,
    &_changeModeCmdHandler,
    &_copyCmdHandler,
    &_pastCmdHandler,
    &_cutCmdHandler,
    &_saveCmdHandler,
    &_clearClipboardCmdHandler,
    &_undoHandler,
    &_recvHandler,
    NULL,   // exitHandler - Обрабатывается в TextEditorCore_exec
    &_outlineHelpCmdHandler,
    &_outlineStateHandler,
    &_timeoutCmdHandler,
};

void Core_init
        ( Core * o,
          const Modules * modules,
          const LPM_EditorSystemParams * systemParams)
{
    o->modules = modules;
    o->display = systemParams->displayDriver;
    o->endlType = systemParams->settings->defaultEndOfLineType;
    o->tabSpaceAmount = systemParams->settings->tabSpaceAmount;
    o->flags = 0;
}


uint32_t Core_exec(Core * o)
{
    _prepare(o);

    for(;;)
    {
        EditorCmd cmd = CmdReader_read(o->modules->cmdReader);

        if(cmd == EDITOR_CMD_EXIT)
            break;
        else if(cmd < __EDITOR_NO_CMD)
            (*(cmdHandlerTable[cmd]))(o);

        if(TextStorage_needToSync(o->modules->textStorage))
            _syncTextStorage(o);
    }

    LPM_UnicodeDisplay_clearScreen(o->display);
    return LPM_EDITOR_OK;
}

void Core_setReadOnly(Core * o)
{
    o->flags |= FLAG_READ_ONLY;
}

void Core_setTemplateMode(Core * o)
{
    o->flags |= FLAG_TEMPLATE_MODE;
}

void Core_setInsertionsMode(Core * o)
{
    o->flags |= FLAG_INSERTIONS_MODE;
}

void Core_checkTemplateFormat(Core * o, uint32_t * badPageMap)
{
    (void)o;
    *badPageMap = 0;
}

bool Core_checkInsertionFormatAndReadNameIfOk(Core * o, uint16_t * templateName)
{
    (void)o, (void)templateName;
    return true;
}

void _prepare(Obj * o)
{
    size_t endOfText = TextStorage_endOfText(o->modules->textStorage);
    o->textCursor.pos = endOfText;
    o->textCursor.len = 0;
    PageFormatter_startWithPageAtTextPosition(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
    _syncTextStorage(o);
}

void _printLineMap(Core * o)
{
    test_print_page_map(  o->modules->pageFormatter->pageStruct.base,
                         &o->modules->pageFormatter->pageStruct.prevLastLine,
                          o->modules->pageFormatter->pageStruct.lineMapTable );
}

void _printDisplayCursor(Core * o)
{
    test_print_display_cursor( o->modules->pageFormatter->displayCursor.begin.x,
                               o->modules->pageFormatter->displayCursor.begin.y,
                               o->modules->pageFormatter->displayCursor.end.x,
                               o->modules->pageFormatter->displayCursor.end.y );
}

void _printTextCursor(Core * o)
{
    test_print_text_cursor(o->textCursor.pos, o->textCursor.len);
}


void _cursorChangedCmdHandler(Core * o)
{
    uint16_t flags = CmdReader_getFlags(o->modules->cmdReader);
    PageFormatter_updatePageWhenCursorMoved(o->modules->pageFormatter, flags, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _textChangedCmdHandler(Core * o)
{
    if(_readOnlyMode(o))
        return;

    uint16_t flag = CmdReader_getFlags(o->modules->cmdReader);
    bool enoughPlaceInTextSrorage = true;

    if(flag == TEXT_FLAG_TEXT)
        enoughPlaceInTextSrorage = _processEnteredChar(o);

    else if(flag == TEXT_FLAG_TAB)
        enoughPlaceInTextSrorage = _processEnteredTab(o);

    else if(flag == TEXT_FLAG_NEW_LINE)
        enoughPlaceInTextSrorage = _processEnteredNewLine(o);

    else if(flag == TEXT_FLAG_INSERTION_BORDER)
        enoughPlaceInTextSrorage = _processEnteredInsertionBorder(o);


    else if(flag == TEXT_FLAG_REMOVE_NEXT_CHAR)
    {
        if(PageFormatter_hasAddChars(o->modules->pageFormatter))
            return;
        _processRemoveNextChar(o);
    }

    else if(flag == TEXT_FLAG_REMOVE_PREV_CHAR)
    {
        if(PageFormatter_hasAddChars(o->modules->pageFormatter))
        {
            PageFormatter_updatePageWhenCursorMoved
                    ( o->modules->pageFormatter,
                      CURSOR_FLAG_MOVE | CURSOR_FLAG_CHAR | CURSOR_FLAG_LEFT,
                      &o->textCursor);
            PageFormatter_updateDisplay(o->modules->pageFormatter);
            return;
        }
        _processRemovePrevChar(o);
    }

    else if(flag == TEXT_FLAG_REMOVE_PAGE)
        enoughPlaceInTextSrorage = _processRemovePage(o);

    else if(flag == TEXT_FLAG_TRUCATE_LINE)
        enoughPlaceInTextSrorage = _processTruncateLine(o);

    if(enoughPlaceInTextSrorage)
    {
        PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
        PageFormatter_updateDisplay(o->modules->pageFormatter);
    }
    else
    {
        _handleNotEnoughPlaceInTextStorage(o);
    }
}

void _changeModeCmdHandler(Core * o)
{
    (void)o;
}

void _copyCmdHandler(Core * o)
{
    if(o->textCursor.len > 0)
        if(!TextBuffer_push(o->modules->clipboardTextBuffer, &o->textCursor))
            _handleNotEnoughPlaceInClipboard(o);
}

void _pastCmdHandler(Core * o)
{
    if(_readOnlyMode(o))
        return;

    if(o->modules->clipboardTextBuffer->usedSize == 0)
        return;

    if(_checkPastSizeWriteAddCharsAndSaveAction(o))
    {
        TextBuffer_pop(o->modules->clipboardTextBuffer, &o->textCursor);
        PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
        PageFormatter_updateDisplay(o->modules->pageFormatter);
    }
    else
    {
        _handleNotEnoughPlaceInTextStorage(o);
    }
}

void _cutCmdHandler(Core * o)
{
    if(_readOnlyMode(o))
        return;

    if(o->textCursor.len > 0)
    {
        if(TextBuffer_push(o->modules->clipboardTextBuffer, &o->textCursor))
        {
            _saveAction(o, 0);
            TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
            o->textCursor.len = 0;
            PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
            PageFormatter_updateDisplay(o->modules->pageFormatter);
        }
        else
        {
            _handleNotEnoughPlaceInClipboard(o);
        }
    }
}

void _clearClipboardCmdHandler(Core * o)
{
    TextBuffer_clear(o->modules->clipboardTextBuffer);
}

void _saveCmdHandler(Core * o)
{
    _syncTextStorage(o);
}

void _undoHandler(Core * o)
{
    _undoAction(o);
    PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _recvHandler(Core * o)
{
    TextStorage_recv(o->modules->textStorage, &o->textCursor);
    _setHasActionToUndo(o, false);
    PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _outlineHelpCmdHandler(Core * o)
{
    ScreenPainter_drawEditorMessage(o->modules->screenPainter, EDITOR_MESSAGE_SHORT_CUTS);
    PageFormatter_updateWholePage(o->modules->pageFormatter);
}

void _outlineStateHandler(Core * o)
{
     PageFormatter_updateWholePage(o->modules->pageFormatter);
}

void _timeoutCmdHandler(Core * o)
{
    (void)o;
}

bool _processEnteredChar(Core * o)
{
    Unicode_Buf textToWrite;
    bool pureDiacritic;
    if(!_checkCharsAndPrepareToWrite(o, &textToWrite, &pureDiacritic))
        return true;

    if(CmdReader_isReplacementMode(o->modules->cmdReader))
    {
        if(pureDiacritic || o->textCursor.len > 0)
            return _enterTextDespiteInsertionMode(o, &textToWrite);

        if(o->textCursor.len == 0)
        {
            const unicode_t * pchr = LineBuffer_LoadText(o->modules, o->textCursor.pos, o->modules->charBuffer.size);
            if(!TextOperator_atEndOfLine(o->modules->textOperator, pchr))
                o->textCursor.len = TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;

            if(_enterTextDespiteInsertionMode(o, &textToWrite))
                return true;

            o->textCursor.len = 0;
        }
        else
        {
            return _enterTextDespiteInsertionMode(o, &textToWrite);
        }
    }
    else
    {
        return _enterTextDespiteInsertionMode(o, &textToWrite);
    }
    return false;
}

bool _processEnteredTab(Core * o)
{
    size_t addCharsAmount   = _insertAddCharsToLineBuffer(o, 0);
    _insertTabSeqToLineBuffer(o, addCharsAmount);
    Unicode_Buf text = { o->modules->lineBuffer.data, addCharsAmount + o->tabSpaceAmount };
    return _enterTextDespiteInsertionMode(o, &text);
}

bool _processEnteredNewLine(Core * o)
{
    Unicode_Buf text;
    _setTextBufToEndlSeq(o, &text);
    return _enterTextDespiteInsertionMode(o, &text);
}

bool _processEnteredInsertionBorder(Core * o)
{
    if(_canEnterInsertionBorderChar(o))
        return _processEnteredChar(o);

    return true;
}

void _processRemoveNextChar(Core * o)
{
    if(o->textCursor.len == 0)
    {
        const unicode_t * pchr = LineBuffer_LoadText(o->modules, o->textCursor.pos, o->modules->charBuffer.size);
        o->textCursor.len = TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;
    }
    _saveAction(o, 0);
    TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
    o->textCursor.len = 0;
}

void _processRemovePrevChar(Core * o)
{
    if(o->textCursor.len == 0)
    {
        if(o->textCursor.pos > 0)
        {
            const unicode_t * pchr = LineBuffer_LoadTextBack(o->modules, o->textCursor.pos, o->modules->charBuffer.size);
            const unicode_t * prev = TextOperator_prevChar(o->modules->textOperator, pchr);
            if(TextOperator_atEndOfLine(o->modules->textOperator, prev))
            {
                o->textCursor.len  = pchr - prev;
                o->textCursor.pos -= o->textCursor.len;
            }
            else
            {
                o->textCursor.pos--;
                o->textCursor.len = 1;
            }
            _saveAction(o, 0);
            TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
            o->textCursor.len = 0;
        }
    }
    else
    {
        _saveAction(o, 0);
        TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
        o->textCursor.len = 0;
    }
}

bool _processRemovePage(Core * o)
{
    size_t currPagePos = PageFormatter_getCurrPagePos(o->modules->pageFormatter);
    size_t currPageLen = PageFormatter_getCurrPageLen(o->modules->pageFormatter);
    if(currPagePos <= o->textCursor.pos)
    {
        Unicode_Buf text;
        size_t offset = PageFormatter_fillPageWithEmptyLines
                (o->modules->pageFormatter, &text, o->endlType);

        SlcCurs removingArea = { currPagePos, currPageLen };

        if(TextStorage_enoughPlace(o->modules->textStorage, &removingArea, text.size))
        {
            o->textCursor.pos = currPagePos;
            o->textCursor.len = currPageLen;
            _saveAction(o, text.size);
            TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
            o->textCursor.pos += offset;
            o->textCursor.len = 0;

            return true;
        }
        return false;

//        o->textCursor.pos = currPagePos;
//        o->textCursor.len = currPageLen;
//        _saveAction(o, 0);
//        TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
//        o->textCursor.len = 0;
    }
    return true;
}

bool _processTruncateLine(Core * o)
{
    size_t currLinePos = PageFormatter_getCurrLinePos(o->modules->pageFormatter);
    size_t currLineLen = PageFormatter_getCurrLineLen(o->modules->pageFormatter);

    if(currLinePos <= o->textCursor.pos)
    {   
        size_t offset = o->textCursor.pos - currLinePos;
        if(offset <= currLineLen)
        {

            Unicode_Buf text;
            _setTextBufToEndlSeq(o, &text);

            size_t prevLen = o->textCursor.len;
            o->textCursor.len = currLineLen - offset;

            if(TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, text.size))
            {
                _saveAction(o, text.size);
                TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
                o->textCursor.len = 0;
                return true;
            }

            o->textCursor.len = prevLen;
            return false;
        }
    }
    return true;
}

void _saveAction(Core * o, size_t insertTextSize)
{
    if(o->textCursor.len > 0)
    {
        if(!TextBuffer_push(o->modules->undoTextBuffer, &o->textCursor))
            return;
    }
    else
    {
        TextBuffer_clear(o->modules->undoTextBuffer);
    }

    o->undoTextCursor.pos = o->textCursor.pos;
    o->undoTextCursor.len = insertTextSize;

    _setHasActionToUndo(o, true);
}

void _undoAction(Core * o)
{
    if(_hasNotActionToUndo(o))
        return;

    if(TextBuffer_isEmpty(o->modules->undoTextBuffer))
    {
        TextStorage_replace(o->modules->textStorage, &o->undoTextCursor, NULL);
        o->textCursor.pos = o->undoTextCursor.pos;
        o->textCursor.len = 0;
        _setHasActionToUndo(o, false);
    }
    else
    {
        if(TextBuffer_pop(o->modules->undoTextBuffer, &o->undoTextCursor))
        {
            o->textCursor.pos = o->undoTextCursor.pos;
            o->textCursor.len = 0;
            TextBuffer_clear(o->modules->undoTextBuffer);
            _setHasActionToUndo(o, false);
        }
        else
        {
            _handleNotEnoughPlaceInTextStorage(o);
        }
    }
}

void _handleNotEnoughPlaceInTextStorage(Core * o)
{
    ScreenPainter_drawEditorMessage(o->modules->screenPainter, EDITOR_MESSAGE_TEXT_BUFFER_FULL);
    PageFormatter_updateWholePage(o->modules->pageFormatter);
    test_beep();
}

void _handleNotEnoughPlaceInClipboard(Core *o)
{
    ScreenPainter_drawEditorMessage(o->modules->screenPainter, EDITOR_MESSAGE_CLIPBOARD_FULL);
    PageFormatter_updateWholePage(o->modules->pageFormatter);
    test_beep();
}

bool _checkCharsAndPrepareToWrite(Core * o, Unicode_Buf * textToWrite, bool * pureDiacritic)
{
    // false, если нет ни одного символа для ввода (не прошли проверку)
    // true, если есть символы для ввода

    size_t prevCharsAmount  = _insertPrevCharsToLineBuffer(o);
    size_t addCharsAmount   = _insertAddCharsToLineBuffer(o, prevCharsAmount);
    size_t inputCharsAmount = _insertInputCharsToLineBuffer(o, prevCharsAmount + addCharsAmount, pureDiacritic);

    if(inputCharsAmount > 0)
    {
        textToWrite->data = o->modules->lineBuffer.data + prevCharsAmount;
        textToWrite->size = addCharsAmount + inputCharsAmount;
        return true;
    }
    return false;
}

size_t _insertPrevCharsToLineBuffer(Core * o)
{
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        o->modules->charBuffer.size
    };
    Buffer_LoadTextBack(o->modules->textStorage, o->textCursor.pos, &buf);
    return buf.size;
}

size_t _insertAddCharsToLineBuffer(Core * o, size_t offset)
{
    if(PageFormatter_hasAddChars(o->modules->pageFormatter))
    {
        // Буфер строки должен вместить предыдущий текст, размером до размера
        //  буфера символа и введенный текст, так же размером до размера буфера
        //  символа. Поэтому максмальное количество добавочных символов - размер
        //  буфера строки минус два размера буфера символа
        Unicode_Buf buf =
        {
            o->modules->lineBuffer.data + offset,
            o->modules->lineBuffer.size - o->modules->charBuffer.size*2,
        };
        if(PageFormatter_fillBuffWithAddChars
                (o->modules->pageFormatter, &buf, o->endlType))
            return buf.size;
    }
    return 0;
}

size_t _insertInputCharsToLineBuffer(Core * o, size_t offset, bool * pureDiacritic)
{
    // Проверяем введенную последовательность символов.
    //  Копируем в буфер строки только те, которые прошли проверку модулем TextOperator

    unicode_t * linePtr = o->modules->lineBuffer.data + offset;
    unicode_t * const lineBegin = linePtr;

    Unicode_Buf tmp;
    CmdReader_getText(o->modules->cmdReader, &tmp);

    const unicode_t * textPtr = tmp.data;
    const unicode_t * const textEnd = textPtr + tmp.size;

    *pureDiacritic = true;
    for( ; textPtr != textEnd; textPtr++)
    {
        bool isDiacritic;
        if(TextOperator_checkInputChar(o->modules->textOperator, *textPtr, linePtr, &isDiacritic))
        {
            *linePtr++ = *textPtr;
            if(!isDiacritic)
                *pureDiacritic = false;
        }
    }

    return linePtr - lineBegin;
}


bool _checkPastSizeWriteAddCharsAndSaveAction(Core * o)
{
    size_t addCharsAmount = PageFormatter_addCharsAmount(o->modules->pageFormatter, o->endlType);
    size_t fullTextSize = addCharsAmount + o->modules->clipboardTextBuffer->usedSize;

    if(!TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, fullTextSize))
        return false;

    Unicode_Buf text =
    {
        o->modules->lineBuffer.data,
        o->modules->lineBuffer.size
    };

    if(PageFormatter_fillBuffWithAddChars
            (o->modules->pageFormatter, &text, o->endlType))
    {
        _saveAction(o, fullTextSize);
        TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
        o->textCursor.pos += text.size;
        o->textCursor.len = 0;
    }
    else
    {
        _saveAction(o, o->modules->clipboardTextBuffer->usedSize);
    }

    return true;
}

void _insertTabSeqToLineBuffer(Core * o, size_t offset)
{
    unicode_t * pchr = o->modules->lineBuffer.data + offset;
    unicode_t * const pend = pchr + o->tabSpaceAmount;
    for( ; pchr != pend; pchr++)
        *pchr = chrSpace;
}

void _setTextBufToEndlSeq(Core * o, Unicode_Buf * text)
{
    text->data = o->modules->lineBuffer.data;

    if(o->endlType == LPM_ENDL_TYPE_CR)
    {
        text->size = 1;
        text->data[0] = chrCr;
    }
    else if(o->endlType == LPM_ENDL_TYPE_LF)
    {
        text->size = 1;
        text->data[0] = chrLf;
    }
    else
    {
        text->size = 2;
        text->data[0] = chrCr;
        text->data[1] = chrLf;
    }
}

bool _enterTextDespiteInsertionMode(Core * o, const Unicode_Buf * text)
{
    if(TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, text->size))
    {
        _saveAction(o, text->size);
        TextStorage_replace(o->modules->textStorage, &o->textCursor, text);
        o->textCursor.pos += text->size;
        o->textCursor.len = 0;
        return true;
    }
    return false;
}

bool _readOnlyMode(Core * o)
{
    return o->flags & FLAG_READ_ONLY;
}

bool _canEnterInsertionBorderChar(Core * o)
{
    return o->flags & FLAG_TEMPLATE_MODE;
}

void _setHasActionToUndo(Core * o, bool state)
{
    if(state)
        o->flags |= FLAG_HAS_ACTION_TO_UNDO;
    else
        o->flags &= ~FLAG_HAS_ACTION_TO_UNDO;
}

bool _hasNotActionToUndo(Core * o)
{
    return !(o->flags & FLAG_HAS_ACTION_TO_UNDO);
}

void _syncTextStorage(Core * o)
{
    test_beep();
    TextStorage_sync
            ( o->modules->textStorage,
              PageFormatter_getCurrPagePos(o->modules->pageFormatter) );
    //test_print_unicode(o->modules->recoveryBuffer->buffer.data, o->modules->recoveryBuffer->buffer.size);
}
