#include "text_editor_core.h"
#include "text_editor_command_reader.h"
#include "text_editor_text_operator.h"
#include "text_editor_page_formatter.h"

#include <string.h>
#include <stdio.h>

typedef TextEditorCore Obj;
typedef LPM_SelectionCursor SlcCurs;
typedef void(*CmdHandler)(TextEditorCore*);

#define CMD_READER_TIMEOUT 2500


static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

static void _prepare(Obj * o);
static void _initArea(Obj * o, SlcCurs * cursor);

static void _cursorChangedCmdHandler(TextEditorCore * o);
static void _textChangedCmdHandler(TextEditorCore * o);
static void _changeModeCmdHandler(TextEditorCore * o);
static void _copyCmdHandler(TextEditorCore * o);
static void _pastCmdHandler(TextEditorCore * o);
static void _cutCmdHandler(TextEditorCore * o);
static void _saveCmdHandler(TextEditorCore * o);
static void _clearClipboardCmdHandler(TextEditorCore * o);
static void _undoHandler(TextEditorCore * o);
static void _outlineHelpCmdHandler(TextEditorCore * o);
static void _outlineStateHandler(TextEditorCore * o);
static void _timeoutCmdHandler(TextEditorCore * o);

static void _printLineMap(TextEditorCore * o);

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

void TextEditorCore_init(TextEditorCore * o, TextEditorModules * modules)
{
    o->modules = modules;
}

extern Unicode_Buf testTextBuf;

void TextEditorCore_exec(TextEditorCore * o)
{
    _prepare(o);

    for(;;)
    {
        TextEditorCmd cmd =
                TextEditorCmdReader_read(o->modules->cmdReader, CMD_READER_TIMEOUT);

        if(cmd == TEXT_EDITOR_CMD_EXIT)
            break;
        else if(cmd < __TEXT_EDITOR_NO_CMD)
            (*(cmdHandlerTable[cmd]))(o);
    }
}

void _prepare(Obj * o)
{
    size_t endOfText = TextEditorTextStorage_endOfText(o->modules->textStorage);
    o->textCursor.pos = endOfText-1;
    o->textCursor.len = 0;
    TextEditorPageFormatter_startWithPageAtTextPosition(o->modules->pageFormatter, o->textCursor.pos);
    TextEditorPageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _initArea(Obj * o, SlcCurs * cursor)
{
    cursor->pos = o->textCursor.pos;
    cursor->len = o->textCursor.len;
}

void _printLineMap(TextEditorCore * o)
{
    printf("begin: %d\n", o->modules->pageFormatter->currPageBase);
    for(int i = 0; i < TEXT_EDITOR_PAGE_LINE_AMOUNT; i++)
        printf("    line %d: full %d, payload %d, rest %d\n",
               i,
               o->modules->pageFormatter->lineMap[i].fullLen,
               o->modules->pageFormatter->lineMap[i].payloadLen,
               o->modules->pageFormatter->lineMap[i].restLen );
    printf("flags: %x\n", o->modules->pageFormatter->lineChangedFlags);
}


void _cursorChangedCmdHandler(TextEditorCore * o)
{
    size_t endOfText = TextEditorTextStorage_endOfText(o->modules->textStorage);
    uint16_t flags = TextEditorCmdReader_getFlags(o->modules->cmdReader);
    if((flags & TEXT_EDITOR_CURSOR_FLAG_MOVE) == TEXT_EDITOR_CURSOR_FLAG_MOVE)
    {
        if((flags & TEXT_EDITOR_CURSOR_FLAG_CHAR) == TEXT_EDITOR_CURSOR_FLAG_CHAR)
        {
            if((flags & (3<<3)) == TEXT_EDITOR_CURSOR_FLAG_DOWN)
            {
                o->textCursor.pos += 1000;
                if(o->textCursor.pos > endOfText)
                    o->textCursor.pos = endOfText;
            }
            if((flags & (3<<3)) == TEXT_EDITOR_CURSOR_FLAG_UP)
            {
                if(o->textCursor.pos < 1000)
                    o->textCursor.pos = 0;
                else
                    o->textCursor.pos -= 1000;
            }

            TextEditorPageFormatter_updatePageByTextChanging(o->modules->pageFormatter, NULL, o->textCursor.pos);
            TextEditorPageFormatter_updateDisplay(o->modules->pageFormatter);
        }
    }
    //_printLineMap(o);
}

void _textChangedCmdHandler(TextEditorCore * o)
{
    Unicode_Buf text;
    SlcCurs area;
    uint16_t flags = TextEditorCmdReader_getFlags(o->modules->cmdReader);

    if(flags == TEXT_EDITOR_TEXT_FLAG_TEXT)
    {
        TextEditorCmdReader_getText(o->modules->cmdReader, &text);
        _initArea(o, &area);
        TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);
        area.len = text.size;
        o->textCursor.pos += text.size;
    }

    if(flags == TEXT_EDITOR_TEXT_FLAG_TAB)
    {
        unicode_t   crlf[5] = { chrSpace, chrSpace, chrSpace, chrSpace, chrSpace };
        text.data = crlf;
        text.size = 5;
        area.pos = o->textCursor.pos;
        area.len = o->textCursor.len;

        TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);

        area.pos = o->textCursor.pos;
        area.len = 5;
        o->textCursor.pos += 5;
        o->textCursor.len  = 0;
    }

    if(flags == TEXT_EDITOR_TEXT_FLAG_NEW_LINE)
    {
        unicode_t   crlf[2] = { chrCr, chrLf };
        text.data = crlf;
        text.size = 2;
        area.pos = o->textCursor.pos;
        area.len = o->textCursor.len;

        TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);

        area.pos = o->textCursor.pos;
        area.len = 2;
        o->textCursor.pos += 2;
        o->textCursor.len  = 0;
    }

    if(flags == TEXT_EDITOR_TEXT_FLAG_REMOVE_PREV_CHAR)
    {
        if(o->textCursor.pos == 0)
            return;
        area.pos = o->textCursor.pos-1;
        area.len = 1;
        TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, NULL);
        o->textCursor.pos--;
    }

    if(flags == TEXT_EDITOR_TEXT_FLAG_REMOVE_NEXT_CHAR)
    {
        if(o->textCursor.pos == 0)
            return;
        area.pos = o->textCursor.pos;
        area.len = 1;
        TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, NULL);
    }

    TextEditorPageFormatter_updatePageByTextChanging(o->modules->pageFormatter, &area, o->textCursor.pos);
    TextEditorPageFormatter_updateDisplay(o->modules->pageFormatter);
    //_printLineMap(o);
}


//void _enterNewLineCmdHandler(TextEditorCore * o)
//{
//    unicode_t   crlf[2] = { chrCr, chrLf };
//    Unicode_Buf text    = { crlf, 2 };
//    SlcCurs     area    = { o->textCursor.pos, o->textCursor.len };

//    TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);

//    area.pos = o->textCursor.pos;
//    area.len = 2;
//    o->textCursor.pos += 2;
//    o->textCursor.len  = 0;

//    TextEditorPageFormatter_updatePageByTextChanging(o->modules->pageFormatter, &area, o->textCursor.pos);
//    TextEditorPageFormatter_updateDisplay(o->modules->pageFormatter);
////    Unicode_Buf text = { .data = &chrLf, .size = 1 };
////    LPM_SelectionCursor area = { .pos = 60000, .len = 0 };
////    TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);
////    area.len = 1;
//////    TextEditorPageFormatter_updatePageByTextChanging(o->modules->pageFormatter, &area, );
//////    TextEditorPageFormatter_updateDisplay(o->modules->pageFormatter);
//}

void _deleteCmdHandler(TextEditorCore * o)
{
    if(o->textCursor.pos == 0)
        return;
    SlcCurs area = { o->textCursor.pos-1, 1 };
    TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, NULL);
    o->textCursor.pos--;
    TextEditorPageFormatter_updatePageByTextChanging(o->modules->pageFormatter, &area, o->textCursor.pos);
    TextEditorPageFormatter_updateDisplay(o->modules->pageFormatter);
}

void _changeModeCmdHandler(TextEditorCore * o) { (void)o; }
void _copyCmdHandler(TextEditorCore * o) { (void)o; }
void _pastCmdHandler(TextEditorCore * o) { (void)o; }
void _cutCmdHandler(TextEditorCore * o) { (void)o; }
void _saveCmdHandler(TextEditorCore * o) { (void)o; }
void _clearClipboardCmdHandler(TextEditorCore * o) { (void)o; }
void _undoHandler(TextEditorCore * o) { (void)o; }
void _outlineHelpCmdHandler(TextEditorCore * o) { (void)o; }
void _outlineStateHandler(TextEditorCore * o) { (void)o; }
void _timeoutCmdHandler(TextEditorCore * o) { (void)o; }
