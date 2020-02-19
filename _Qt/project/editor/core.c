#include "core.h"
#include "command_reader.h"
#include "lpm_text_storage.h"
#include "page_formatter.h"
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

void test_print_cursor(size_t bx, size_t by, size_t ex, size_t ey);
void test_print_page_map(size_t base, const LineMap * prev, const LineMap * table);

static void _prepare(Obj * o);
static void _initArea(Obj * o, SlcCurs * cursor);

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

static void _printLineMap(Core * o);
static void _printCursor(Core * o);

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
    _printLineMap(o);

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
    o->textCursor.pos = 0;
    o->textCursor.len = 0;
    PageFormatter_startWithPageAtTextPosition(o->modules->pageFormatter, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);

    unicode_t buf[20];
    Unicode_Buf ub = { buf, 20};
    LPM_TextStorage_read(o->modules->textStorage, 0, &ub);
    LPM_Lang lg;
    LPM_Lang_init(&lg, LPM_LANG_RUS_ENG);
}

void _initArea(Obj * o, SlcCurs * cursor)
{
    cursor->pos = o->textCursor.pos;
    cursor->len = o->textCursor.len;
}

void _printLineMap(Core * o)
{
    test_print_page_map(  o->modules->pageFormatter->currPageBase,
                         &o->modules->pageFormatter->prevPageLastLineMap,
                          o->modules->pageFormatter->lineMap );
}

void _printCursor(Core * o)
{
    test_print_cursor( o->modules->pageFormatter->displayCursor.begin.x,
                       o->modules->pageFormatter->displayCursor.begin.y,
                       o->modules->pageFormatter->displayCursor.end.x,
                       o->modules->pageFormatter->displayCursor.end.y );
}


void _cursorChangedCmdHandler(Core * o)
{
    size_t endOfText = LPM_TextStorage_endOfText(o->modules->textStorage);
    uint16_t flags = CmdReader_getFlags(o->modules->cmdReader);
    if((flags & CURSOR_FLAG_MOVE) == CURSOR_FLAG_MOVE)
    {
        if((flags & CURSOR_FLAG_CHAR) == CURSOR_FLAG_CHAR)
        {
            if((flags & (3<<3)) == CURSOR_FLAG_DOWN)
            {
                o->textCursor.pos += 1000;
                if(o->textCursor.pos > endOfText)
                    o->textCursor.pos = endOfText;
            }
            if((flags & (3<<3)) == CURSOR_FLAG_UP)
            {
                if(o->textCursor.pos < 1000)
                    o->textCursor.pos = 0;
                else
                    o->textCursor.pos -= 1000;
            }

            PageFormatter_updatePageByTextChanging(o->modules->pageFormatter, NULL, &o->textCursor);
            PageFormatter_updateDisplay(o->modules->pageFormatter);
        }
    }
    _printLineMap(o);
}

void _textChangedCmdHandler(Core * o)
{
    Unicode_Buf text;
    SlcCurs area;
    uint16_t flags = CmdReader_getFlags(o->modules->cmdReader);

    if(flags == TEXT_FLAG_TEXT)
    {
        CmdReader_getText(o->modules->cmdReader, &text);
        _initArea(o, &area);
        LPM_TextStorage_replace(o->modules->textStorage, &area, &text);
        area.len = text.size;
        o->textCursor.pos += text.size;
    }

    if(flags == TEXT_FLAG_TAB)
    {
        unicode_t   crlf[5] = { chrSpace, chrSpace, chrSpace, chrSpace, chrSpace };
        text.data = crlf;
        text.size = 5;
        area.pos = o->textCursor.pos;
        area.len = o->textCursor.len;

        LPM_TextStorage_replace(o->modules->textStorage, &area, &text);

        area.pos = o->textCursor.pos;
        area.len = 5;
        o->textCursor.pos += 5;
        o->textCursor.len  = 0;
    }

    if(flags == TEXT_FLAG_NEW_LINE)
    {
        unicode_t   crlf[2] = { chrCr, chrLf };
        text.data = crlf;
        text.size = 2;
        area.pos = o->textCursor.pos;
        area.len = o->textCursor.len;

        LPM_TextStorage_replace(o->modules->textStorage, &area, &text);

        area.pos = o->textCursor.pos;
        area.len = 2;
        o->textCursor.pos += 2;
        o->textCursor.len  = 0;
    }

    if(flags == TEXT_FLAG_REMOVE_PREV_CHAR)
    {
        if(o->textCursor.pos == 0)
            return;
        area.pos = o->textCursor.pos-1;
        area.len = 1;
        LPM_TextStorage_replace(o->modules->textStorage, &area, NULL);
        o->textCursor.pos--;
    }

    if(flags == TEXT_FLAG_REMOVE_NEXT_CHAR)
    {
        if(o->textCursor.pos == 0)
            return;
        area.pos = o->textCursor.pos;
        area.len = 1;
        LPM_TextStorage_replace(o->modules->textStorage, &area, NULL);
    }

    PageFormatter_updatePageByTextChanging(o->modules->pageFormatter, &area, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
    //_printLineMap(o);
}

void _deleteCmdHandler(Core * o)
{
    if(o->textCursor.pos == 0)
        return;
    SlcCurs area = { o->textCursor.pos-1, 1 };
    LPM_TextStorage_replace(o->modules->textStorage, &area, NULL);
    o->textCursor.pos--;
    PageFormatter_updatePageByTextChanging(o->modules->pageFormatter, &area, &o->textCursor);
    PageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _changeModeCmdHandler(Core * o) { (void)o; }
void _copyCmdHandler(Core * o) { (void)o; }
void _pastCmdHandler(Core * o) { (void)o; }
void _cutCmdHandler(Core * o) { (void)o; }
void _saveCmdHandler(Core * o) { (void)o; }
void _clearClipboardCmdHandler(Core * o) { (void)o; }
void _undoHandler(Core * o) { (void)o; }
void _outlineHelpCmdHandler(Core * o) { (void)o; }
void _outlineStateHandler(Core * o) { (void)o; }
void _timeoutCmdHandler(Core * o) { (void)o; }
