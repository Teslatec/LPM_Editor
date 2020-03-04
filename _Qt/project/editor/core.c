#include "core.h"
#include "command_reader.h"
#include "lpm_text_storage.h"
#include "page_formatter.h"
#include "lpm_text_operator.h"
#include "lpm_text_buffer.h"
#include "lpm_lang.h"
#include "line_buffer_support.h"

#include <string.h>
#include <stdio.h>

typedef Core Obj;
typedef LPM_SelectionCursor SlcCurs;
typedef void(*CmdHandler)(Core*);

#define CMD_READER_TIMEOUT 2500
#define CHAR_BUF_SIZE 10


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
static void _copyTextCursor(SlcCurs * dst, const SlcCurs * src);

static void _cursorChangedCmdHandler(Core * o);
static void _textChangedCmdHandler(Core * o);
static void _changeModeCmdHandler(Core * o);
static void _copyCmdHandler(Core * o);
static void _pastCmdHandler(Core * o);
static void _cutCmdHandler(Core * o);
static void _saveCmdHandler(Core * o);
static void _clearClipboardCmdHandler(Core * o);
static void _undoHandler(Core * o);
static void _outlineHelpCmdHandler(Core * o);
static void _outlineStateHandler(Core * o);
static void _timeoutCmdHandler(Core * o);

static bool _processEnteredChar(Core * o);
static bool _processEnteredTab(Core * o);
static bool _processEnteredNewLine(Core * o);
static void _processRemoveNextChar(Core * o);
static void _processRemovePrevChar(Core * o);
static void _processRemovePage(Core * o);
static void _processTruncateLine(Core * o);

static void _saveAction(Core * o, size_t insertTextSize);
static void _undoAction(Core * o);

static void _handleNotEnoughPlaceInTextStorage(Core * o);
static void _handleNotEnoughPlaceInClipboard(Core * o);

static bool _checkInputText(Core * o, Unicode_Buf * text);

static void _printLineMap(Core * o);
static void _printDisplayCursor(Core * o);
static void _printTextCursor(Core * o);

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
    NULL,   // exitHandler - Обрабатывается в TextEditorCore_exec
    &_outlineHelpCmdHandler,
    &_outlineStateHandler,
    &_timeoutCmdHandler,
};

void Core_init(Core * o, Modules * modules, LPM_EndOfLineType endOfLineType)
{
    o->modules = modules;
    o->endOfLineType = endOfLineType;
    o->hasActionToUndo = false;
}

extern Unicode_Buf testTextBuf;

void Core_exec(Core * o)
{
    _prepare(o);

    for(;;)
    {
        EditorCmd cmd =
                CmdReader_read(o->modules->cmdReader, CMD_READER_TIMEOUT);

        if(cmd == EDITOR_CMD_EXIT)
            break;
        else if(cmd < __EDITOR_NO_CMD)
            (*(cmdHandlerTable[cmd]))(o);
    }

    LPM_UnicodeDisplay_clearScreen(o->modules->display);
}

void _prepare(Obj * o)
{
    size_t endOfText = LPM_TextStorage_endOfText(o->modules->textStorage);
    o->textCursor.pos = endOfText;
    o->textCursor.len = 0;
    PageFormatter_startWithPageAtTextPosition(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _copyTextCursor(SlcCurs * dst, const SlcCurs * src)
{
    dst->pos = src->pos;
    dst->len = src->len;
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
    uint16_t flag = CmdReader_getFlags(o->modules->cmdReader);
    bool enoughPlaceInTextSrorage = true;

    if(flag == TEXT_FLAG_TEXT)
        enoughPlaceInTextSrorage = _processEnteredChar(o);

    else if(flag == TEXT_FLAG_TAB)
        enoughPlaceInTextSrorage = _processEnteredTab(o);

    else if(flag == TEXT_FLAG_NEW_LINE)
        enoughPlaceInTextSrorage = _processEnteredNewLine(o);


    else if(flag == TEXT_FLAG_REMOVE_NEXT_CHAR)
        _processRemoveNextChar(o);

    else if(flag == TEXT_FLAG_REMOVE_PREV_CHAR)
        _processRemovePrevChar(o);

    else if(flag == TEXT_FLAG_REMOVE_PAGE)
        _processRemovePage(o);

    else if(flag == TEXT_FLAG_TRUCATE_LINE)
        _processTruncateLine(o);

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
        if(!LPM_TextBuffer_push(o->modules->clipboardTextBuffer, &o->textCursor))
            _handleNotEnoughPlaceInClipboard(o);
}

void _pastCmdHandler(Core * o)
{
    if(LPM_TextBuffer_checkPlaceInTextStorage(o->modules->clipboardTextBuffer, &o->textCursor))
    {
        _saveAction(o, o->modules->clipboardTextBuffer->usedSize);
        LPM_TextBuffer_pop(o->modules->clipboardTextBuffer, &o->textCursor);
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
    if(o->textCursor.len > 0)
    {
        if(LPM_TextBuffer_push(o->modules->clipboardTextBuffer, &o->textCursor))
        {
            _saveAction(o, 0);
            LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
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
    LPM_TextBuffer_clear(o->modules->clipboardTextBuffer);
}

void _saveCmdHandler(Core * o)
{
    LPM_TextStorage_sync(o->modules->textStorage);
}

void _undoHandler(Core * o)
{
    _undoAction(o);
    PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _outlineHelpCmdHandler(Core * o) { (void)o; }
void _outlineStateHandler(Core * o) { (void)o; }

void _timeoutCmdHandler(Core * o)
{
    LPM_TextStorage_sync(o->modules->textStorage);
}

bool _processEnteredChar(Core * o)
{
    Unicode_Buf text;
    CmdReader_getText(o->modules->cmdReader, &text);
    if(!_checkInputText(o, &text))
        return true;

    if(CmdReader_isReplacementMode(o->modules->cmdReader))
    {
        if(o->textCursor.len == 0)
        {
            const unicode_t * pchr = LineBuffer_LoadText(o->modules, o->textCursor.pos, CHAR_BUF_SIZE);
            o->textCursor.len = LPM_TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;

            if(LPM_TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, &text))
            {
                _saveAction(o, text.size);
                LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
                o->textCursor.pos += text.size;
                o->textCursor.len = 0;
                return true;
            }
            o->textCursor.len = 0;
        }
        else
        {
            if(LPM_TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, &text))
            {
                _saveAction(o, text.size);
                LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
                o->textCursor.pos += text.size;
                o->textCursor.len = 0;
                return true;
            }
        }
    }
    else
    {
        if(LPM_TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, &text))
        {
            _saveAction(o, text.size);
            LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
            o->textCursor.pos += text.size;
            o->textCursor.len = 0;
            return true;
        }
    }
    return false;
}

bool _processEnteredTab(Core * o)
{
    static const unicode_t tabArray[5] = { 0x0020, 0x0020, 0x0020, 0x0020, 0x0020 };
    Unicode_Buf text = { (unicode_t*)tabArray, 5 };

    if(LPM_TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, &text))
    {
        _saveAction(o, text.size);
        LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
        o->textCursor.pos += text.size;
        o->textCursor.len = 0;
        return true;
    }
    return false;
}

bool _processEnteredNewLine(Core * o)
{
    static const unicode_t endlArray[2] = { 0x000D, 0x000A };
    Unicode_Buf text;
    if(o->endOfLineType == LPM_END_OF_LINE_TYPE_CR)
    {
        text.data = (unicode_t*)endlArray;
        text.size = 1;
    }
    else if(o->endOfLineType == LPM_END_OF_LINE_TYPE_LF)
    {
        text.data = (unicode_t*)endlArray+1;
        text.size = 1;
    }
    else
    {
        text.data = (unicode_t*)endlArray;
        text.size = 2;
    }

    if(LPM_TextStorage_enoughPlace(o->modules->textStorage, &o->textCursor, &text))
    {
        _saveAction(o, text.size);
        LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
        o->textCursor.pos += text.size;
        o->textCursor.len = 0;
        return true;
    }
    return false;
}

void _processRemoveNextChar(Core * o)
{
    if(o->textCursor.len == 0)
    {
        const unicode_t * pchr = LineBuffer_LoadText(o->modules, o->textCursor.pos, CHAR_BUF_SIZE);
        o->textCursor.len = LPM_TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;
    }
    _saveAction(o, 0);
    LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
    o->textCursor.len = 0;
}

void _processRemovePrevChar(Core * o)
{
    if(o->textCursor.len == 0)
    {
        if(o->textCursor.pos > 0)
        {
            const unicode_t * pchr = LineBuffer_LoadTextBack(o->modules, o->textCursor.pos, CHAR_BUF_SIZE);
            const unicode_t * prev = LPM_TextOperator_prevChar(o->modules->textOperator, pchr);
            if(LPM_TextOperator_atEndOfLine(o->modules->textOperator, prev))
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
            LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
            o->textCursor.len = 0;
        }
    }
    else
    {
        _saveAction(o, 0);
        LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
        o->textCursor.len = 0;
    }
}

void _processRemovePage(Core * o)
{
    size_t currPagePos = PageFormatter_getCurrPagePos(o->modules->pageFormatter);
    size_t currPageLen = PageFormatter_getCurrPageLen(o->modules->pageFormatter);
    if(currPagePos <= o->textCursor.pos)
    {
        o->textCursor.pos = currPagePos;
        o->textCursor.len = currPageLen;
        _saveAction(o, 0);
        LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
        o->textCursor.len = 0;
    }
}

void _processTruncateLine(Core * o)
{
    size_t currLinePos = PageFormatter_getCurrLinePos(o->modules->pageFormatter);
    size_t currLineLen = PageFormatter_getCurrLineLen(o->modules->pageFormatter);

    if(currLinePos <= o->textCursor.pos)
    {
        size_t offset = o->textCursor.pos - currLinePos;
        if(offset <= currLineLen)
        {
            o->textCursor.pos = currLinePos + offset;
            o->textCursor.len = currLineLen - offset;
            _saveAction(o, 0);
            LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, NULL);
            o->textCursor.len = 0;
        }
    }
}

void _saveAction(Core * o, size_t insertTextSize)
{
    if(o->textCursor.len > 0)
    {
        if(!LPM_TextBuffer_push(o->modules->undoTextBuffer, &o->textCursor))
            return;
    }
    else
    {
        LPM_TextBuffer_clear(o->modules->undoTextBuffer);
    }

    o->undoTextCursor.pos = o->textCursor.pos;
    o->undoTextCursor.len = insertTextSize;
    o->hasActionToUndo = true;
}

void _undoAction(Core * o)
{
    if(!o->hasActionToUndo)
        return;

    if(LPM_TextBuffer_isEmpty(o->modules->undoTextBuffer))
    {
        LPM_TextStorage_replace(o->modules->textStorage, &o->undoTextCursor, NULL);
        o->textCursor.pos = o->undoTextCursor.pos;
        o->textCursor.len = 0;
        o->hasActionToUndo = false;
    }
    else
    {
        if(LPM_TextBuffer_pop(o->modules->undoTextBuffer, &o->undoTextCursor))
        {
            o->textCursor.pos = o->undoTextCursor.pos;
            o->textCursor.len = 0;
            LPM_TextBuffer_clear(o->modules->undoTextBuffer);
            o->hasActionToUndo = false;
        }
        else
        {
            _handleNotEnoughPlaceInTextStorage(o);
        }
    }
}

void _handleNotEnoughPlaceInTextStorage(Core * o)
{
    // TODO: вывод сообщения о недостатке места в буфере текста
    (void)o;
    test_beep();
}

void _handleNotEnoughPlaceInClipboard(Core *o)
{
    (void)o;
    // TODO: вывод сообщения о недостатке места в буфере текста
    test_beep();
}

bool _checkInputText(Core * o, Unicode_Buf * text)
{
    unicode_t * const inputBegin = LineBuffer_LoadTextBack(o->modules, o->textCursor.pos, CHAR_BUF_SIZE);
    unicode_t * inputPtr = inputBegin;
    const unicode_t * textPtr = text->data;
    const unicode_t * const textEnd = textPtr + text->size;

    for( ; textPtr != textEnd; textPtr++)
        if(LPM_TextOperator_checkInputChar(o->modules->textOperator, *textPtr, inputPtr))
            *inputPtr++ = *textPtr;

    text->data = inputBegin;
    text->size = inputPtr - inputBegin;
    return text->size != 0;
}
