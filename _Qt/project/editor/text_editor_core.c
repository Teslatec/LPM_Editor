#include "text_editor_core.h"
#include "text_editor_command_reader.h"
#include "text_editor_text_operator.h"

#include <string.h>

#define CMD_READER_TIMEOUT 2500

static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

static void _prepare(TextEditorCore * o);

static void _rebuildLineTableWhenTextChanged( TextEditorCore * o,
                                              const LPM_SelectionCursor * chagedTextArea );
static void _buildLineTableFromPos(TextEditorCore * o, size_t pos);

static void _updateDispayData(TextEditorCore * o);
static void _updateDisplayCursorByTextCursor();
static void _moveDisplayCursorAndUpdateTextCursor();
static void _prepareLineForDisplay();

static size_t _calcLineMapAndModifyChangedFlag( TextEditorCore * o,
                                                size_t lineNewBegin,
                                                size_t lineIndex ,
                                                const LPM_SelectionCursor * chagedTextArea );
static bool _isLineChanged( const TextEditorLineTableItem * lineMap,
                            size_t newLineBegin,
                            const LPM_SelectionCursor * chagedTextArea );
static bool _areaIntersect( size_t firstBegin,
                            size_t firstEnd,
                            size_t secondBegin,
                            size_t secondEnd );
static void _writeLineChangedFlag( TextEditorCore * o,
                                   size_t lineIndex,
                                   bool value );
static size_t _updateLineMap( TextEditorCore * core,
                              TextEditorLineTableItem * lineMap,
                              size_t lineNewBegin );

static void _readLineFromTextToLineBuffer( TextEditorCore * core,
                                           size_t pos );

static bool _checkForLineTermination( const unicode_t * pchr,
                                      TextEditorLineTableItem * lineMap,
                                      size_t chrCurrPos,
                                      size_t * nextLineBeginPos );

static bool _chrIsDiacritic(unicode_t chr);

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

    for(int i = 0; i < 16; i++)
        printf("line %d: %d, %d\n", i, o->pageMap.lineTable[i].begin,
               o->pageMap.lineTable[i].end );
}


void _rebuildLineTableWhenTextChanged( TextEditorCore * o,
                       const LPM_SelectionCursor * chagedTextArea )
{
    size_t currLineBegin = o->pageMap.lineTable[0].begin;
    for( size_t lineIndex = 0;
         lineIndex < TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT;
         lineIndex++ )
        currLineBegin = _calcLineMapAndModifyChangedFlag( o,
                                                          currLineBegin,
                                                          lineIndex,
                                                          chagedTextArea );
}

void _buildLineTableFromPos(TextEditorCore * o, size_t pos)
{
    TextEditorLineTableItem * lineMap = o->pageMap.lineTable;
    for( size_t lineIndex = 0;
         lineIndex < TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT;
         lineIndex++, lineMap++ )
    {
        _writeLineChangedFlag(o, lineIndex, true);
        pos = _updateLineMap(o, lineMap, pos);
    }
}

void _updateDispayData(TextEditorCore * o)
{}

size_t _calcLineMapAndModifyChangedFlag( TextEditorCore * o,
                                         size_t lineNewBegin,
                                         size_t lineIndex,
                                         const LPM_SelectionCursor * chagedTextArea )
{

    TextEditorLineTableItem * lineMap = o->pageMap.lineTable + lineIndex;
    if(_isLineChanged(lineMap, lineNewBegin, chagedTextArea))
    {
        _writeLineChangedFlag(o, lineIndex, true);
        return _updateLineMap(o, lineMap, lineNewBegin);
    }

    _writeLineChangedFlag(o, lineIndex, false);
    return o->pageMap.lineTable[++lineIndex].begin;
}

bool _isLineChanged( const TextEditorLineTableItem * lineMap,
                     size_t newLineBegin,
                     const LPM_SelectionCursor * chagedTextArea )
{
    if(newLineBegin != lineMap->begin)
        return true;

    if(chagedTextArea == NULL)
        return false;

    if(_areaIntersect( lineMap->begin,
                       lineMap->end,
                       chagedTextArea->pos,
                       chagedTextArea->pos+chagedTextArea->len ))
        return true;

    return false;
}

bool _areaIntersect( size_t firstBegin,
                     size_t firstEnd,
                     size_t secondBegin,
                     size_t secondEnd )
{
    if(secondBegin == secondEnd)
        return false;

    if(secondBegin >= firstEnd)
        return false;

    if(secondEnd <= firstBegin)
        return false;

    return true;
}

void _writeLineChangedFlag( TextEditorCore * o,
                            size_t lineIndex,
                            bool value )
{
    if(value)
        o->pageMap.changedLineFlags |=  (1u << lineIndex);
    else
        o->pageMap.changedLineFlags &= ~(1u << lineIndex);
}

size_t _updateLineMap( TextEditorCore * core,
                       TextEditorLineTableItem * lineMap,
                       size_t lineNewBegin )
{
    lineMap->begin = lineNewBegin;
    _readLineFromTextToLineBuffer(core, lineNewBegin);

    const size_t lineBufSize = core->modules->lineBuffer.size;
    size_t chrPlaceCount = 0;
    size_t chrCurrPos = lineNewBegin;
    size_t lastSpacePos = lineBufSize;
    const unicode_t * pchr = core->modules->lineBuffer.data;
    const unicode_t * end   = pchr + lineBufSize;

    for( ; pchr != end; pchr++, chrCurrPos++)
    {
        if(chrPlaceCount == TEXT_EDITOR_PAGE_MAP_CHAR_AMOUNT)
            break;

        if(_checkForLineTermination(pchr, lineMap, chrCurrPos, &lineNewBegin))
            return lineNewBegin;

        if(_chrIsDiacritic(*pchr))
            continue;

        if(*pchr == chrSpace)
            lastSpacePos = chrCurrPos;

        chrPlaceCount++;
    }

    if(lastSpacePos == chrCurrPos-1)
    {
        lineMap->end = chrCurrPos;
        return lineMap->end;
    }

    if( (*pchr == chrEndOfText) ||
            (*pchr == chrLf) ||
            (*pchr == chrCr) ||
            (*pchr == chrSpace) )
    {
        lineMap->end = chrCurrPos+1;
        return lineMap->end;
    }

    lineMap->end = lastSpacePos;
    return lineMap->end;
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

bool _checkForLineTermination( const unicode_t * pchr,
                               TextEditorLineTableItem * lineMap,
                               size_t chrCurrPos,
                               size_t * nextLineBeginPos )
{
    if(*pchr == chrEndOfText)
    {
        lineMap->end = chrCurrPos;
        *nextLineBeginPos = lineMap->end;
        return true;
    }

    if(*pchr == chrLf)
    {
        lineMap->end = chrCurrPos;
        *nextLineBeginPos = chrCurrPos+1;
        return true;
    }

    if(*pchr == chrCr)
    {
        lineMap->end = chrCurrPos;
        ++pchr;
        *nextLineBeginPos = *pchr == chrLf ? chrCurrPos+2 : chrCurrPos+1;
        return true;
    }

    return false;
}

bool _chrIsDiacritic(unicode_t chr)
{
    return Unicode_getSymType(chr) == UNICODE_SYM_TYPE_DIACRITIC;
}

void _moveCursorCmdHandler(TextEditorCore * o) { (void)o; }
void _changeSelectionCmdHandler(TextEditorCore * o) { (void)o; }
void _enterSymbolCmdHandler(TextEditorCore * o) { (void)o; }
void _enterTabCmdHandler(TextEditorCore * o) { (void)o; }
void _enterNewLineCmdHandler(TextEditorCore * o) { (void)o; }
void _deleteCmdHandler(TextEditorCore * o) { (void)o; }
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
