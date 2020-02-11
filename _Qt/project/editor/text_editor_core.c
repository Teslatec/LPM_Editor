#include "text_editor_core.h"
#include "text_editor_command_reader.h"
#include "text_editor_text_operator.h"

#include <string.h>
#include <stdio.h>

#define CMD_READER_TIMEOUT 2500

static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

static void _prepare(TextEditorCore * o);

static void _rebuildLineTableWhenTextChanged( TextEditorCore * o,
                                              const LPM_SelectionCursor * changedTextArea );
static void _buildLineTableFromPos(TextEditorCore * o, size_t pos);

static void _updateDispayData(TextEditorCore * o);
static void _updateDisplayCursorByTextCursor();
static void _moveDisplayCursorAndUpdateTextCursor();

static void _updateLineMap( TextEditorCore * o,
                            TextEditorLineMap * lineMap,
                            uint16_t lineOffset );

static bool _areaIntersect( size_t firstBegin,
                            size_t firstEnd,
                            size_t secondBegin,
                            size_t secondEnd );

static void _readLineFromTextToLineBuffer( TextEditorCore * core,
                                           size_t pos );

static bool _chrIsDiacritic(unicode_t chr);

static void _formatLineForDisplay( TextEditorCore * o,
                                   const TextEditorLineMap * lineMap,
                                   size_t lineOffset );

static bool _changedTextCrossesLine( const LPM_SelectionCursor * changedTextArea,
                                     size_t lineMapBegin,
                                     size_t lineMapLen );

static bool _textChangedBeforeEmptyLine( const LPM_SelectionCursor * changedTextArea,
                                         size_t lineMapBegin );

static void _setLineChangedFlag(TextEditorCore * o, size_t lineIndex);
static void _clearLineChangedFlag(TextEditorCore * o, size_t lineIndex);
static bool _readLineChangedFlag(TextEditorCore * o, size_t lineIndex);
static void _clearAllLineChangedFlags(TextEditorCore * o);
static void _setAllLineChangedFlags(TextEditorCore * o);

static void _printLineMap(TextEditorCore * o);

typedef void(*CmdHandler)(TextEditorCore*);

static void _moveCursorCmdHandler(TextEditorCore * o);
static void _changeSelectionCmdHandler(TextEditorCore * o);
static void _enterSymbolCmdHandler(TextEditorCore * o);
static void _enterTabCmdHandler(TextEditorCore * o);
static void _enterNewLineCmdHandler(TextEditorCore * o);
static void _deleteCmdHandler(TextEditorCore * o);
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

static const CmdHandler cmdHandlerTable[] =
{
    &_moveCursorCmdHandler,
    &_changeSelectionCmdHandler,
    &_enterSymbolCmdHandler,
    &_enterTabCmdHandler,
    &_enterNewLineCmdHandler,
    &_deleteCmdHandler,
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

    _buildLineTableFromPos(o, 710);
    _printLineMap(o);
    _updateDispayData(o);

    for(;;)
    {
        TextEditorCmd cmd =
                TextEditorCmdReader_read(o->modules->cmdReader, CMD_READER_TIMEOUT);

        if(cmd == TEXT_EDITOR_CMD_EXIT)
            break;
        else if(cmd < __TEXT_EDITOR_NO_CMD)
            (*(cmdHandlerTable[cmd]))(o);

//        _buildLineTableFromPos(o, 710);
//        _printLineMap(o);
//        _updateDispayData(o);

//        LPM_SelectionCursor area = { .pos = 710, .len = 445 };
//        unicode_t text[7] = { 'A','A','A','!',' ','!', '!' };
//        Unicode_Buf textBuf = { .data = text, .size = 5 };
//        TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &testTextBuf);
//        //area.len = testTextBuf.size;
//        _rebuildLineTableWhenTextChanged(o, &area);
//        _printLineMap(o);
//        _updateDispayData(o);

//        _buildLineTableFromPos(o, 710);
//        _printLineMap(o);
//        _updateDispayData(o);
//        break;
    }

    // В общем случае в обработчиках:
    // Обновить текст
    // Обновить карту
    // Обновить курсор
    // Обновить дисплей
}

void _prepare(TextEditorCore * o)
{
    memset(&o->pageMap, 0, sizeof(TextEditorPageMap));
    //o->pageMap.pageAmount = ...;
    _buildLineTableFromPos(o, 0);
    _updateDispayData(o);
    _printLineMap(o);
}


void _rebuildLineTableWhenTextChanged( TextEditorCore * o,
                                       const LPM_SelectionCursor * changedTextArea )
{
    uint16_t expectedLineOffset = 0;
    uint16_t actualLineOffset = 0;
    TextEditorLineMap * lineMap = o->pageMap.lineTable;
    TextEditorLineMap * end = o->pageMap.lineTable + TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT;

    _clearAllLineChangedFlags(o);

    for(size_t lineIndex = 0; lineMap != end ; lineIndex++, lineMap++)
    {
        if(lineMap->fullLen == 0)
        {
            if(_textChangedBeforeEmptyLine( changedTextArea,
                                            o->pageMap.currPageBegin + actualLineOffset ) ||
                    (actualLineOffset != expectedLineOffset) )
            {
                expectedLineOffset = actualLineOffset + lineMap->fullLen;
                _updateLineMap(o, lineMap, actualLineOffset);
                actualLineOffset += lineMap->fullLen;
                if(lineMap->fullLen > 0)
                    _setLineChangedFlag(o, lineIndex);
            }
            else
            {
                expectedLineOffset = actualLineOffset + lineMap->fullLen;
                actualLineOffset += lineMap->fullLen;
            }
        }
        else
        {
            if( _changedTextCrossesLine( changedTextArea,
                                         o->pageMap.currPageBegin + actualLineOffset,
                                         lineMap->fullLen ) ||
                    (actualLineOffset != expectedLineOffset) )
            {
                expectedLineOffset = actualLineOffset + lineMap->fullLen;
                _updateLineMap(o, lineMap, actualLineOffset);
                actualLineOffset += lineMap->fullLen;
                _setLineChangedFlag(o, lineIndex);
            }
            else
            {
                expectedLineOffset = actualLineOffset + lineMap->fullLen;
                actualLineOffset += lineMap->fullLen;
            }
        }
    }
    o->pageMap.nextPageBegin = o->pageMap.currPageBegin + actualLineOffset;
}

void _buildLineTableFromPos(TextEditorCore * o, size_t pos)
{
    o->pageMap.currPageBegin = pos;
    _setAllLineChangedFlags(o);
    TextEditorLineMap * lineMap = o->pageMap.lineTable;
    TextEditorLineMap * end = o->pageMap.lineTable + TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT;
    uint16_t offset = 0;
    for( ; lineMap != end; lineMap++)
    {
        _updateLineMap(o, lineMap, offset);
        offset += lineMap->fullLen;
    }
    o->pageMap.nextPageBegin = o->pageMap.currPageBegin + offset;
}

void _updateDispayData(TextEditorCore * o)
{
    LPM_Point pos = { .x = 0, .y = 0 };
    Unicode_Buf lineBuf = { .data = o->modules->lineBuffer.data, .size = 0 };
    const TextEditorLineMap * lineMap = o->pageMap.lineTable;
    const TextEditorLineMap * end = o->pageMap.lineTable + TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT;
    LPM_UnicodeDisplay_setCursor(o->modules->display, &o->pageMap.displayCursor);
    size_t lineOffset = o->pageMap.currPageBegin;
    size_t lineIndex = 0;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {
        if(_readLineChangedFlag(o, lineIndex))
        {
            _formatLineForDisplay(o, lineMap, lineOffset);
            lineBuf.size = lineMap->payloadLen + lineMap->restLen;
            LPM_UnicodeDisplay_writeLine(o->modules->display, &lineBuf, &pos);
            printf("updated line %d\n", lineIndex);
        }
        pos.y++;
        lineOffset += lineMap->fullLen;
    }
}

bool _changedTextCrossesLine( const LPM_SelectionCursor * changedTextArea,
                              size_t lineMapBegin,
                              size_t lineMapLen )
{
    if(changedTextArea == NULL)
        return false;

    return _areaIntersect( lineMapBegin,
                           lineMapBegin + lineMapLen,
                           changedTextArea->pos,
                           changedTextArea->pos + changedTextArea->len );
}

bool _textChangedBeforeEmptyLine( const LPM_SelectionCursor * changedTextArea,
                                  size_t lineMapBegin )
{
    if(changedTextArea == NULL)
        return false;

    return lineMapBegin <= changedTextArea->pos;
}

bool _areaIntersect( size_t firstBegin,
                     size_t firstEnd,
                     size_t secondBegin,
                     size_t secondEnd )
{
    if(secondBegin == secondEnd)
        return false;

    if(secondBegin > firstEnd)
        return false;

    if(secondEnd <= firstBegin)
        return false;

    return true;
}

void _updateLineMap( TextEditorCore * o,
                     TextEditorLineMap * lineMap,
                     uint16_t lineOffset )
{
    _readLineFromTextToLineBuffer(o, o->pageMap.currPageBegin + lineOffset);

    const size_t lineBufSize = o->modules->lineBuffer.size;
    const size_t chrAmount = TEXT_EDITOR_PAGE_MAP_CHAR_AMOUNT;
    size_t chrPlaceCount = 0;
    size_t chrPlaceCountAtLastSpaceDetected = 0;
    size_t chrCurrPos = 0;
    size_t lastSpacePos = lineBufSize;
    const unicode_t * pchr = o->modules->lineBuffer.data;
    const unicode_t * end   = pchr + lineBufSize;

    for( ; pchr != end; pchr++, chrCurrPos++)
    {
        if(chrPlaceCount == chrAmount)
            break;

        if(*pchr == chrEndOfText)
        {
            lineMap->fullLen    = chrCurrPos;
            lineMap->payloadLen = chrCurrPos;
            lineMap->restLen    = chrAmount - chrPlaceCount;
            return;
        }

        if(*pchr == chrLf)
        {
            lineMap->fullLen    = chrCurrPos+1;
            lineMap->payloadLen = chrCurrPos;
            lineMap->restLen    = chrAmount - chrPlaceCount;
            return;
        }

        if(*pchr == chrCr)
        {
            ++pchr;
            lineMap->fullLen    = chrCurrPos + (*pchr == chrLf ? 2 : 1);
            lineMap->payloadLen = chrCurrPos;
            lineMap->restLen    = chrAmount - chrPlaceCount;
            return;
        }

        if(_chrIsDiacritic(*pchr))
            continue;

        if(*pchr == chrSpace)
        {
            lastSpacePos = chrCurrPos;
            chrPlaceCountAtLastSpaceDetected = chrPlaceCount;
        }

        chrPlaceCount++;
    }

    if(*pchr == chrEndOfText)
    {
        lineMap->fullLen    = chrCurrPos;
        lineMap->payloadLen = chrCurrPos;
        lineMap->restLen    = 0;
        return;
    }

    if(*pchr == chrLf)
    {
        lineMap->fullLen    = chrCurrPos+1;
        lineMap->payloadLen = chrCurrPos;
        lineMap->restLen    = 0;
        return;
    }

    if(*pchr == chrCr)
    {
        ++pchr;
        lineMap->fullLen    = chrCurrPos + (*pchr == chrLf ? 2 : 1);
        lineMap->payloadLen = chrCurrPos;
        lineMap->restLen    = 0;
        return;
    }

    if(*pchr == chrSpace)
    {
        lineMap->payloadLen = chrCurrPos;
        lineMap->restLen = 0;
        ++pchr;
        if( (*pchr == chrCr) )
        {
            ++pchr;
            if( (*pchr == chrLf) )
            {
                lineMap->fullLen = chrCurrPos+3;
                return;
            }
            lineMap->fullLen = chrCurrPos+2;
            return;
        }
        lineMap->fullLen = chrCurrPos+1;
        return;
    }

    lineMap->payloadLen = lastSpacePos;
    lineMap->restLen    = chrAmount - chrPlaceCountAtLastSpaceDetected;
    lineMap->fullLen    = lastSpacePos+1;
    return;
}

void _readLineFromTextToLineBuffer(TextEditorCore * core, size_t pos)
{
    Unicode_Buf buf =
    {
        .data = core->modules->lineBuffer.data,
        .size = core->modules->lineBuffer.size,
    };

    TextEditorTextOperator_read(core->modules->textOperator, pos, &buf);
    if(buf.size < core->modules->lineBuffer.size)
        memset( buf.data + buf.size, 0,
                (core->modules->lineBuffer.size - buf.size)*sizeof(unicode_t) );
}

bool _chrIsDiacritic(unicode_t chr)
{
    return Unicode_getSymType(chr) == UNICODE_SYM_TYPE_DIACRITIC;
}

void _formatLineForDisplay( TextEditorCore * o,
                            const TextEditorLineMap * lineMap,
                            size_t lineOffset )
{
    if(lineMap->payloadLen > 0)
    {

        Unicode_Buf buf =
        {
            .data = o->modules->lineBuffer.data,
            .size = lineMap->payloadLen,
        };
        TextEditorTextOperator_read(o->modules->textOperator, lineOffset, &buf);
    }

    unicode_t * pchr = o->modules->lineBuffer.data + lineMap->payloadLen;
    unicode_t * const end = pchr + lineMap->restLen;
    for( ; pchr != end; pchr++)
        *pchr = chrSpace;
}

void _setLineChangedFlag(TextEditorCore * o, size_t lineIndex)
{
    o->pageMap.changedLineFlags |=  (1u << lineIndex);
}

void _clearLineChangedFlag(TextEditorCore * o, size_t lineIndex)
{
    o->pageMap.changedLineFlags &= ~(1u << lineIndex);
}

bool _readLineChangedFlag(TextEditorCore * o, size_t lineIndex)
{
    return o->pageMap.changedLineFlags & (1u << lineIndex);
}

void _clearAllLineChangedFlags(TextEditorCore * o)
{
    o->pageMap.changedLineFlags = 0x0000;
}

void _setAllLineChangedFlags(TextEditorCore * o)
{
    o->pageMap.changedLineFlags = 0xFFFF;
}

void _printLineMap(TextEditorCore * o)
{
    printf("begin: %d\n", o->pageMap.currPageBegin);
    for(int i = 0; i < TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT; i++)
        printf("    line %d: full %d, payload %d, rest %d\n",
               i,
               o->pageMap.lineTable[i].fullLen,
               o->pageMap.lineTable[i].payloadLen,
               o->pageMap.lineTable[i].restLen );
    printf("next: %d\n", o->pageMap.nextPageBegin);
}


void _moveCursorCmdHandler(TextEditorCore * o) { (void)o; }
void _changeSelectionCmdHandler(TextEditorCore * o) { (void)o; }

void _enterSymbolCmdHandler(TextEditorCore * o)
{
    Unicode_Buf text;
    TextEditorCmdReader_getText(o->modules->cmdReader, &text);
    LPM_SelectionCursor area = { .pos = 2000, .len = 0 };
    TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);
    area.len = 1;
    _rebuildLineTableWhenTextChanged(o, &area);
    _updateDispayData(o);
}

void _enterTabCmdHandler(TextEditorCore * o) { (void)o; }

void _enterNewLineCmdHandler(TextEditorCore * o)
{
    Unicode_Buf text = { .data = &chrLf, .size = 1 };
    LPM_SelectionCursor area = { .pos = 2000, .len = 0 };
    TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, &text);
    area.len = 1;
    _rebuildLineTableWhenTextChanged(o, &area);
    _updateDispayData(o);
}

void _deleteCmdHandler(TextEditorCore * o)
{
    size_t endOfText = TextEditorTextStorage_endOfText(o->modules->textStorage);
    LPM_SelectionCursor area = { .pos = endOfText-1, .len = 1 };
    TextEditorTextOperator_removeAndWrite(o->modules->textOperator, &area, NULL);
    _rebuildLineTableWhenTextChanged(o, &area);
    _updateDispayData(o);
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
