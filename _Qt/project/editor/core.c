#include "core.h"
#include "command_reader.h"
#include "lpm_text_storage.h"
#include "page_formatter.h"
#include "lpm_text_operator.h"
#include "lpm_lang.h"

#include <string.h>
#include <stdio.h>

typedef Core Obj;
typedef LPM_SelectionCursor SlcCurs;
typedef void(*CmdHandler)(Core*);

#define CMD_READER_TIMEOUT 2500


static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

void test_print_display_cursor(size_t bx, size_t by, size_t ex, size_t ey);
void test_print_text_cursor(size_t pos, size_t len);
void test_print_page_map(size_t base, const LineMap * prev, const LineMap * table);

static void _prepare(Obj * o);
static void _copyTextCursor(const SlcCurs * src, SlcCurs * dst);

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

static void _processEnteredChar(Core * o);

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

void Core_init(Core * o, Modules * modules)
{
    o->modules = modules;
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
}

void _prepare(Obj * o)
{
    size_t endOfText = LPM_TextStorage_endOfText(o->modules->textStorage);
    o->textCursor.pos = endOfText;
    o->textCursor.len = 0;
    PageFormatter_startWithPageAtTextPosition(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _copyTextCursor(const SlcCurs * src, SlcCurs * dst)
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
    if(flag == TEXT_FLAG_TEXT)
        _processEnteredChar(o);

    PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);

//    Unicode_Buf text;
//    SlcCurs removingArea;
//    _copyTextCursor(&o->textCursor, &removingArea);
//    uint16_t flags = CmdReader_getFlags(o->modules->cmdReader);
//    bool isReplaceMode = CmdReader_isReplacementMode(o->modules->cmdReader);

//    if(flags == TEXT_FLAG_TEXT)
//    {
//        if(removingArea.len > 0)
//        {
//            LPM_TextStorage_replace(o->modules->textStorage, &removingArea, NULL);
//            removingArea.len = 0;
//            CmdReader_getText(o->modules->cmdReader, &text);
//            LPM_TextStorage_replace(o->modules->textStorage, &removingArea, &text);
//            o->textCursor.pos += text.size;
//            o->textCursor.len = 0;
//        }

//        CmdReader_getText(o->modules->cmdReader, &text);
//        if(isReplaceMode)
//            if(removingArea.len == 0)
//                removingArea.len = 1;
//        LPM_TextStorage_replace(o->modules->textStorage, &removingArea, &text);
//        o->textCursor.pos += text.size;
//        o->textCursor.len = 0;
//    }

//    if(flags == TEXT_FLAG_TAB)
//    {
//        unicode_t   crlf[5] = { chrSpace, chrSpace, chrSpace, chrSpace, chrSpace };
//        text.data = crlf;
//        text.size = 5;
//        removingArea.pos = o->textCursor.pos;
//        removingArea.len = o->textCursor.len;

//        LPM_TextStorage_replace(o->modules->textStorage, &removingArea, &text);

//        removingArea.pos = o->textCursor.pos;
//        removingArea.len = 5;
//        o->textCursor.pos += 5;
//        o->textCursor.len  = 0;
//    }

//    if(flags == TEXT_FLAG_NEW_LINE)
//    {
//        unicode_t   crlf[2] = { chrCr, chrLf };
//        text.data = crlf;
//        text.size = 2;
//        removingArea.pos = o->textCursor.pos;
//        removingArea.len = o->textCursor.len;

//        LPM_TextStorage_replace(o->modules->textStorage, &removingArea, &text);

//        removingArea.pos = o->textCursor.pos;
//        removingArea.len = 2;
//        o->textCursor.pos += 2;
//        o->textCursor.len  = 0;
//    }

//    if(flags == TEXT_FLAG_REMOVE_PREV_CHAR)
//    {
//        if(o->textCursor.pos == 0)
//            return;
//        removingArea.pos = o->textCursor.pos-1;
//        removingArea.len = 1;
//        LPM_TextStorage_replace(o->modules->textStorage, &removingArea, NULL);
//        o->textCursor.pos--;
//    }

//    if(flags == TEXT_FLAG_REMOVE_NEXT_CHAR)
//    {
//        if(o->textCursor.pos == 0)
//            return;
//        removingArea.pos = o->textCursor.pos;
//        removingArea.len = 1;
//        LPM_TextStorage_replace(o->modules->textStorage, &removingArea, NULL);
//    }

//    PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
//    PageFormatter_updateDisplay(o->modules->pageFormatter);
    //_printLineMap(o);
}

void _changeModeCmdHandler(Core * o)
{
    PageFormatter_updatePageWhenTextChanged(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _copyCmdHandler(Core * o) { (void)o; }
void _pastCmdHandler(Core * o) { (void)o; }
void _cutCmdHandler(Core * o) { (void)o; }

void _clearClipboardCmdHandler(Core * o) { (void)o; }

void _saveCmdHandler(Core * o) { (void)o; }

void _undoHandler(Core * o) { (void)o; }

void _outlineHelpCmdHandler(Core * o) { (void)o; }
void _outlineStateHandler(Core * o) { (void)o; }
void _timeoutCmdHandler(Core * o) { (void)o; }

void _processEnteredChar(Core * o)
{
    Unicode_Buf text;
    CmdReader_getText(o->modules->cmdReader, &text);
    if(_checkInputText(o, &text))
    {
        LPM_TextStorage_replace(o->modules->textStorage, &o->textCursor, &text);
        o->textCursor.pos += text.size;
        if(o->textCursor.len > 0)
        {
            o->textCursor.len = 0;
//            if(CmdReader_isReplacementMode(o->modules->cmdReader))
//                ; // Удалить следующий символ
        }
    }
}

bool _checkInputText(Core * o, Unicode_Buf * text)
{
    // Проверка символа:
    // 1. Загрузить в буфер строки N предыдущих символов
    // 2. В цикле:
    //  2.1. Вызвать функцию проверки модуля LPM_TextOperator
    //  2.2. Если прошел проверку, записать из буфера текста в буфер строки
    // 3. Записать указатель на новый текст (который лежит в буфере строки),
    //    а также его размер, в структуру text

    const size_t chrAmount = 10;
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        o->textCursor.pos < chrAmount ? o->textCursor.pos : chrAmount
    };
    test_print("!", buf.data, 42);
    LPM_TextStorage_read(o->modules->textStorage, o->textCursor.pos-buf.size, &buf);
    test_print("!", buf.data[0], 42);
    unicode_t * pchr = o->modules->lineBuffer.data+buf.size;//_loadPrevChars(o, chrAmount);
    unicode_t * newchr = pchr;
    size_t i;
    for(i = 0; i < text->size; i++, newchr++)
        if(LPM_TextOperator_checkInputChar(o->modules->textOperator, text->data[i], newchr))
            *newchr++ = text->data[i];
        else
            break;
    text->data = pchr;
    text->size = i;
    return i != 0;

//    const size_t chrAmount = 10;
//    unicode_t * pchr = _loadPrevChars(o, chrAmount);
//    unicode_t * const base = pchr + chrAmount;
//    while()

//    if(o->textCursor.pos > 0)
//    {

//        if(pos > 0)
//        {
//            size_t loadLen = pos < 10 ? pos : 10;
//            unicode_t * pchr = _loadLine(o, pos-loadLen, loadLen) + loadLen;
//            pos -= pchr - LPM_TextOperator_prevChar(o->modules->textOperator, pchr);
//        }
//    }
//    LPM_TextOperator_checkInputChar(o->modules->textOperator, c, pchr);
//    return true;
}
