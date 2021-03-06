#include "screen_painter.h"
#include "command_reader.h"
#include "text_operator.h"
#include "lpm_unicode_display.h"
#include "page_formatter.h"
#include <string.h>

typedef ScreenPainter Obj;

static const unicode_t chrLineH   = 0x2500;
static const unicode_t chrLineV   = 0x2502;
static const unicode_t chrAngleLU = 0x250C;
static const unicode_t chrAngleRU = 0x2510;
static const unicode_t chrAngleLD = 0x2514;
static const unicode_t chrAngleRD = 0x2518;
static const unicode_t chrSpace   = 0x0020;

static const unicode_t * _editorMessageToTextPointer(Obj * o, EditorMessage msg);
static void _drawText(Obj * o, const unicode_t * text);
static void _waitForAnyKeyPressed(Obj * o);
static void _drawLineBuffer(Obj * o, size_t size, size_t lineIndex);
static size_t _formatBorderLine(Obj * o, bool isUp);
static void _drawBorderLine(Obj *o, bool isUp);
static void _drawTextWithBorderLine(Obj * o, const unicode_t * ptxt, size_t textSize, size_t lineIndex);
static size_t _formatTextWithBorderLine(Obj * o, const unicode_t * ptxt, size_t textSize);

void ScreenPainter_drawEditorMessage
    ( ScreenPainter * o,
      EditorMessage msg )
{
    const unicode_t * text = _editorMessageToTextPointer(o, msg);
    _drawText(o, text);
    _waitForAnyKeyPressed(o);
}

uint32_t ScreenPainter_drawTemplateFormatErrors
    ( ScreenPainter * o,
      uint32_t badPageMap )
{
    (void)o; (void)badPageMap;
    return LPM_EDITOR_OK;
}

uint32_t ScreenPainter_selectTemplateName
    ( ScreenPainter * o, uint8_t * templateIndex )
{
    *templateIndex = 0;
    (void)o;
    return LPM_EDITOR_OK;
}

uint32_t ScreenPainter_createTemplateName
    ( ScreenPainter * o, uint8_t * templateIndex )
{
    *templateIndex = 0;
    (void)o;
    return LPM_EDITOR_OK;
}

const unicode_t * _editorMessageToTextPointer(Obj * o, EditorMessage msg)
{
    uint32_t tableAddr = (uint32_t)o->textTable;
    return *(const unicode_t**)(tableAddr + sizeof(const unicode_t*) * (size_t)msg);
}

void _drawText(Obj * o, const unicode_t * text)
{
    LPM_UnicodeDisplay_clearScreen(o->display);

    _drawBorderLine(o, true);

    LPM_TextLineMap lineMap;
    const unicode_t * currLine = text;
    bool lastLine = false;
    for(size_t i = 1; i < o->pageParams->lineAmount-1u; i++)
    {
        if(!lastLine)
        {
            lastLine = TextOperator_analizeLine
                    ( o->modules->textOperator,
                      currLine,
                      o->pageParams->charAmount-2,
                      &lineMap );

            size_t textSize = lineMap.printBorder-currLine;
            _drawTextWithBorderLine(o, currLine, textSize, i);
            currLine = lineMap.nextLine;
        }
        else
        {
            _drawTextWithBorderLine(o, NULL, 0, i);
        }
    }

    _drawBorderLine(o, false);
}

void _waitForAnyKeyPressed(Obj * o)
{
    while(CmdReader_read(o->modules->cmdReader) == EDITOR_CMD_TIMEOUT) {}
}

void _drawLineBuffer(Obj * o, size_t size, size_t lineIndex)
{
    Unicode_Buf buf = { o->modules->lineBuffer.data, size };
    LPM_SelectionCursor curs = { size, 0 };
    LPM_UnicodeDisplay_writeLine(o->display, lineIndex, &buf, &curs);
}

size_t _formatBorderLine(Obj * o, bool isUp)
{
    unicode_t * pchr = o->modules->lineBuffer.data;
    unicode_t * const end = o->modules->lineBuffer.data + o->pageParams->charAmount-1;
    *pchr++ = isUp ? chrAngleLU : chrAngleLD;
    for( ; pchr != end; pchr++)
        *pchr = chrLineH;
    *pchr = isUp ? chrAngleRU : chrAngleRD;
    return o->pageParams->charAmount;
}

void _drawBorderLine(Obj *o, bool isUp)
{
    size_t drawSize = _formatBorderLine(o, isUp);
    _drawLineBuffer(o, drawSize, isUp ? 0 : o->pageParams->lineAmount-1);
}

void _drawTextWithBorderLine(Obj * o, const unicode_t * ptxt, size_t textSize, size_t lineIndex)
{
    size_t drawSize = _formatTextWithBorderLine(o, ptxt, textSize);
    _drawLineBuffer(o, drawSize, lineIndex);
}

size_t _formatTextWithBorderLine(Obj * o, const unicode_t * ptxt, size_t textSize)
{
    size_t restSize = o->pageParams->charAmount- 2 - textSize;
    unicode_t * pchr = o->modules->lineBuffer.data;
    *pchr++ = chrLineV;
    memcpy(pchr, ptxt, textSize*sizeof(unicode_t));
    pchr += textSize;
    unicode_t * const end = pchr + restSize;
    for( ; pchr != end; pchr++)
        *pchr = chrSpace;
    *pchr = chrLineV;
    return textSize+restSize+2;
}
