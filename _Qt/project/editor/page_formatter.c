#include "page_formatter.h"
#include "lpm_text_storage.h"
#include "lpm_text_operator.h"
#include "crc16_table.h"
#include "editor_flags.h"

#include <string.h>

// TODO: разобратья с граничным условием: глюки при редактировнии последней страницы!!!!

#define LINE_AMOUNT PAGE_LINE_AMOUNT
#define CHAR_AMOUNT PAGE_CHAR_AMOUNT
#define LAST_GROUP  (PAGE_GROUP_AMOUNT-1)

typedef PageFormatter Obj;
typedef LPM_DisplayCursor DspCurs;
typedef LPM_SelectionCursor SlcCurs;

typedef enum TextPos
{
    TEXT_POS_BEFORE_CURRENT_PAGE = 0,
    TEXT_POS_ON_CURRENT_PAGE,
    TEXT_POS_AFTER_CURRENT_PAGE
} TextPos;

static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

static void _updatePageByTextCursor(Obj * o, const SlcCurs * curs);

static void _setFirstPageInGroup(Obj * o, size_t groupIndex);
static void _setNextPage(Obj * o);
static bool _changePageIfNotOnCurrTextPosition(Obj * o, const SlcCurs * pos);

static void _switchToFirstPageInGroup(Obj * o, size_t groupIndex);
static void _switchToNextPage(Obj * o);
static size_t _calcNextPageBase(Obj * o);
static size_t _calcCurrPageLen(Obj * o);
static size_t _calcCurrPageFirstLineBase(Obj * o);
static bool _incPageIndex(Obj * o);
static size_t _findGroupWithNearestOffset(Obj * o, size_t pos);

static void _updateLinesMap(Obj * o);

static TextPos _checkTextPos(Obj * o, size_t pos);
static bool _isCurrPageFirst(Obj * o);
static bool _isCurrPageLast(Obj * o);

static size_t _updateLineMap(Obj * o, LineMap * lineMap, size_t lineBase);

static void _copyLineMap(LineMap * dst, const LineMap * src);
static bool _lineChanged(const LineMap * before, const LineMap * after);
static uint16_t _calcLineCrc(Obj * o, LineMap * lineMap);
static void _makePrevLineMapForFirstPage(LineMap * lineMap);

static unicode_t * _loadLine(Obj * o, size_t loadPos, size_t loadSize);

static void _setAllLineChangedFlags(Obj * o);
static void _resetAllLineChangedFlags(Obj * o);
static bool _readLineChangedFlag(Obj * o, size_t lineIndex);
static void _setLineChangedFlag(Obj * o, size_t lineIndex);

static bool _changedTextCrossesLine( const LPM_SelectionCursor * changedTextArea,
                                     size_t lineMapBegin,
                                     size_t lineMapLen );

static bool _areaIntersect( size_t firstBegin,
                            size_t firstEnd,
                            size_t secondBegin,
                            size_t secondEnd );

static bool _posWithinRange(size_t pos, size_t rangeBegin, size_t rangeEnd);

static void _textCursorToDisplayCursor(Obj * o, const SlcCurs * curs, DspCurs * dspCurs);
static void _displayCursorToLineCursor(Obj * o, size_t lineIndex, size_t lineSize, SlcCurs * lineCursor);

static bool _moveFlagsToTextCursor(Obj * o, uint32_t moveFlags, SlcCurs * textCurs);
static void _moveCursorToLineBorder(Obj * o, uint32_t borderFlag, SlcCurs * textCurs);
static bool _moveCursorToPageBorder(Obj * o, uint32_t borderFlag, SlcCurs * textCurs);
static void _moveCursorAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs);

static void _selectAllPage(Obj * o, SlcCurs * textCurs);
static void _selectAllLine(Obj * o, SlcCurs * textCurs);
static void _changeSelectionAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs);

static uint32_t _getTypeFlagValue(uint32_t flags);
static uint32_t _getGoalFlagValue(uint32_t flags);
static uint32_t _getDirectionFlagValue(uint32_t flags);
static uint32_t _getBorderFlagValue(uint32_t flags);

static void _saveDisplayCursor(Obj * o, const DspCurs * dspCurs);
static void _updateLineChangedFlagsWhenDisplayCursorChanged(Obj * o, DspCurs const * newLineCurs);


static void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset);

void PageFormatter_init
        ( PageFormatter * o,
          const Modules * modules )
{
    memset(o, 0, sizeof(PageFormatter));
    o->modules = modules;
}

void PageFormatter_startWithPageAtTextPosition
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs )
{
    DspCurs dspCurs;
    _setFirstPageInGroup(o, 0);
    _changePageIfNotOnCurrTextPosition(o, curs);
    _textCursorToDisplayCursor(o, curs, &dspCurs);
    _saveDisplayCursor(o, &dspCurs);
}

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs )
{
    _resetAllLineChangedFlags(o);
    _updateLinesMap(o);
    _updatePageByTextCursor(o, curs);
}


void PageFormatter_updatePageWhenCursorMoved
    ( PageFormatter * o,
      uint32_t moveFlags,
      LPM_SelectionCursor * textCurs )
{
    //_resetAllLineChangedFlags(o);
    _setAllLineChangedFlags(o);
    if(!_moveFlagsToTextCursor(o, moveFlags, textCurs))
        _updatePageByTextCursor(o, textCurs);
}

void PageFormatter_updateDisplay
        ( PageFormatter * o )
{
    Unicode_Buf lineBuf = { o->modules->lineBuffer.data, 0 };
    LPM_SelectionCursor lineCursor;

    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * end     = lineMap + LINE_AMOUNT;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    size_t lineIndex  = 0;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {

        if(_readLineChangedFlag(o, lineIndex))
        {
            lineBuf.size = lineMap->payloadLen + lineMap->restLen;
            _displayCursorToLineCursor(o, lineIndex, lineBuf.size, &lineCursor);
            _formatLineForDisplay(o, lineMap, lineBase);
            LPM_UnicodeDisplay_writeLine( o->modules->display,
                                          lineIndex,
                                          &lineBuf,
                                          &lineCursor);
        }
        lineBase += lineMap->fullLen;
    }
}



void _updatePageByTextCursor(Obj * o, const SlcCurs * curs)
{
    DspCurs dspCurs;
    if(!_changePageIfNotOnCurrTextPosition(o, curs))
    {
        _textCursorToDisplayCursor(o, curs, &dspCurs);
        _updateLineChangedFlagsWhenDisplayCursorChanged(o, &dspCurs);
    }
    else
    {
        _textCursorToDisplayCursor(o, curs, &dspCurs);
    }
    _saveDisplayCursor(o, &dspCurs);
}

void _setFirstPageInGroup(Obj * o, size_t groupIndex)
{
    _switchToFirstPageInGroup(o, groupIndex);
    _setAllLineChangedFlags(o);
    _updateLinesMap(o);
}

void _setNextPage(Obj * o)
{
    if(_isCurrPageLast(o))
    {
        _resetAllLineChangedFlags(o);
        return;
    }
    _switchToNextPage(o);
    _setAllLineChangedFlags(o);
    _updateLinesMap(o);
}

bool _changePageIfNotOnCurrTextPosition(Obj * o, const SlcCurs * pos)
{
    TextPos tp = _checkTextPos(o, pos->pos);

    if(tp == TEXT_POS_ON_CURRENT_PAGE)
        return false;

    if(tp == TEXT_POS_AFTER_CURRENT_PAGE)
    {
        _setNextPage(o);
    }
    else
    {
        size_t groupIndex = _findGroupWithNearestOffset(o, pos->pos);
        _setFirstPageInGroup(o, groupIndex);
    }

    while(_checkTextPos(o, pos->pos) != TEXT_POS_ON_CURRENT_PAGE)
        _setNextPage(o);
    return true;
}


void _switchToFirstPageInGroup(Obj * o, size_t groupIndex)
{
    o->pageNavi.currPageIndex  = 0;
    o->pageNavi.currGroupIndex = groupIndex;
    o->pageStruct.base      = o->pageNavi.groupBaseTable[groupIndex];
}

void _switchToNextPage(Obj * o)
{
    o->pageStruct.base = _calcNextPageBase(o);
    if(_incPageIndex(o))
        o->pageNavi.groupBaseTable[o->pageNavi.currGroupIndex] =
                o->pageStruct.base;
}

size_t _calcNextPageBase(Obj * o)
{
    size_t base = _calcCurrPageFirstLineBase(o);
    const LineMap * lineMap   = o->pageStruct.lineMapTable;
    const LineMap * const end = o->pageStruct.lineMapTable+LINE_AMOUNT-1;
    for( ; lineMap != end; lineMap++)
        base += lineMap->fullLen;
    return base;
}

size_t _calcCurrPageLen(Obj * o)
{
    size_t len = 0;
    const LineMap * lineMap   = o->pageStruct.lineMapTable;
    const LineMap * const end = o->pageStruct.lineMapTable+LINE_AMOUNT;
    for( ; lineMap != end; lineMap++)
        len += lineMap->fullLen;
    return len;
}

size_t _calcCurrPageFirstLineBase(Obj * o)
{
    return o->pageStruct.base + o->pageStruct.prevLastLine.fullLen;
}

bool _incPageIndex(Obj * o)
{
    o->pageNavi.currPageIndex++;
    if(o->pageNavi.currGroupIndex < LAST_GROUP)
    {
        if(o->pageNavi.currPageIndex == PAGES_IN_GROUP)
        {
            o->pageNavi.currPageIndex = 0;
            o->pageNavi.currGroupIndex++;
            return true;
        }
    }
    return false;
}

size_t _findGroupWithNearestOffset(Obj * o, size_t pos)
{
    size_t * poffs = o->pageNavi.groupBaseTable + o->pageNavi.currGroupIndex;
    size_t * const pend = o->pageNavi.groupBaseTable;
    size_t groupIndex = o->pageNavi.currGroupIndex;
    for( ; poffs != pend; poffs--, groupIndex--)
        if(*poffs < pos)
            break;
    return groupIndex;
}

void _updateLinesMap(Obj * o)
{
    o->pageStruct.lastPageReached = false;

//    printf("build\n");

    size_t lineBase = o->pageStruct.base;
    if(!_isCurrPageFirst(o))
        lineBase += _updateLineMap(o, &o->pageStruct.prevLastLine, lineBase);
    else
        _makePrevLineMapForFirstPage(&o->pageStruct.prevLastLine);

    LineMap * lineMap   = o->pageStruct.lineMapTable;
    LineMap * const end = o->pageStruct.lineMapTable + LINE_AMOUNT;
    LineMap copy;
    size_t lineIndex = 0;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {
        _copyLineMap(&copy, lineMap);
        lineBase += _updateLineMap(o, lineMap, lineBase);
        if(_lineChanged(lineMap, &copy))
            _setLineChangedFlag(o, lineIndex);
    }

//    printf("flags: %x\n", o->lineChangedFlags);
}

TextPos _checkTextPos(Obj * o, size_t pos)
{
    if(_isCurrPageFirst(o))
    {
        return pos < _calcCurrPageLen(o) ?
                    TEXT_POS_ON_CURRENT_PAGE :
                    TEXT_POS_AFTER_CURRENT_PAGE;
    }

    if(_isCurrPageLast(o))
    {
        return pos >= _calcCurrPageFirstLineBase(o) ?
                    TEXT_POS_ON_CURRENT_PAGE :
                    TEXT_POS_BEFORE_CURRENT_PAGE;
    }

    size_t currPageBegin = _calcCurrPageFirstLineBase(o);
    size_t currPageEnd   = currPageBegin + _calcCurrPageLen(o);

    if(pos < currPageBegin)
        return TEXT_POS_BEFORE_CURRENT_PAGE;

    if(pos >= currPageEnd)
        return TEXT_POS_AFTER_CURRENT_PAGE;

    return TEXT_POS_ON_CURRENT_PAGE;
}

bool _isCurrPageFirst(Obj * o)
{
    return (o->pageNavi.currPageIndex == 0) &&
            (o->pageNavi.currGroupIndex == 0);
}

bool _isCurrPageLast(Obj * o)
{
    return o->pageStruct.lastPageReached;
}


static size_t _updateLineMap(Obj * o, LineMap * lineMap, size_t lineBase)
{
    const unicode_t * const begin =
            _loadLine(o, lineBase, o->modules->lineBuffer.size);

    LPM_TextLineMap textLineMap;
    if(LPM_TextOperator_analizeLine( o->modules->textOperator,
                                     begin,
                                     CHAR_AMOUNT,
                                     &textLineMap) )
        o->pageStruct.lastPageReached = true;

    lineMap->fullLen    = (uint8_t)(textLineMap.nextLine    - begin);
    lineMap->payloadLen = (uint8_t)(textLineMap.printBorder - begin);
    lineMap->restLen    = CHAR_AMOUNT - textLineMap.lenInChr;
    lineMap->crc        = _calcLineCrc(o, lineMap);
    return lineMap->fullLen;
}

void _copyLineMap(LineMap * dst, const LineMap * src)
{
    memcpy(dst, src, sizeof(LineMap));
}

bool _lineChanged(const LineMap * before, const LineMap * after)
{
    if(before->restLen == after->restLen)
        if(before->fullLen == after->fullLen)
            if(before->restLen == after->restLen)
                if(before->crc == after->crc)
                    return false;
    return true;
}

uint16_t _calcLineCrc(Obj * o, LineMap * lineMap)
{
    return crc16_table_calc_for_array(
                (uint8_t*)(o->modules->lineBuffer.data),
                lineMap->payloadLen * sizeof(unicode_t) );
}

void _makePrevLineMapForFirstPage(LineMap * lineMap)
{
    memset(lineMap, 0, sizeof(LineMap));
}

unicode_t * _loadLine(Obj * o, size_t loadPos, size_t loadSize)
{
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        //o->modules->lineBuffer.size
        loadSize
    };
    //size_t fullSize = o->modules->lineBuffer.size;
    size_t fullSize = loadSize;
    LPM_TextStorage_read(o->modules->textStorage, loadPos, &buf);
    memset(buf.data + buf.size, 0, (fullSize - buf.size)*sizeof(unicode_t));
    return o->modules->lineBuffer.data;
}

void _setAllLineChangedFlags(Obj * o)
{
    o->lineChangedFlags = 0xFFFF;
}

void _resetAllLineChangedFlags(Obj * o)
{
    o->lineChangedFlags = 0;
}

bool _readLineChangedFlag(Obj * o, size_t lineIndex)
{
    return o->lineChangedFlags & (1 << lineIndex);
}

void _setLineChangedFlag(Obj * o, size_t lineIndex)
{
    o->lineChangedFlags |= (1 << lineIndex);
}

bool _changedTextCrossesLine( const LPM_SelectionCursor * changedTextArea,
                              size_t lineBase,
                              size_t lineFullLen )
{
    if(changedTextArea == NULL)
        return false;

    return _areaIntersect( lineBase,
                           lineBase + lineFullLen,
                           changedTextArea->pos,
                           changedTextArea->pos + changedTextArea->len );
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

bool _posWithinRange(size_t pos, size_t rangeBegin, size_t rangeEnd)
{
    return (pos >= rangeBegin) && (pos < rangeEnd);
}

void _textCursorToDisplayCursor(Obj * o, const SlcCurs * curs, DspCurs * dspCurs)
{
    dspCurs->begin.y = LINE_AMOUNT;
    dspCurs->end.y   = LINE_AMOUNT;
    size_t findPos = curs->pos;
    LPM_Point * findPoint = &dspCurs->begin;

    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * const end = lineMap + LINE_AMOUNT;
    size_t lineIndex = 0;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    for( ; lineMap != end; lineIndex++, lineMap++)
    {
        lineBase += lineMap->fullLen;

        if(lineMap->fullLen == 0)
        {
            if(lineMap == o->pageStruct.lineMapTable)
            {
                dspCurs->begin.x = 0;
                dspCurs->begin.y = 0;
                dspCurs->end.x = 0;
                dspCurs->end.y = 0;
            }
            else
            {
                --lineMap;
                --lineIndex;
                dspCurs->begin.x = lineMap->fullLen;
                dspCurs->begin.y = lineIndex;
                dspCurs->end.x = lineMap->fullLen;
                dspCurs->end.y = lineIndex;
            }
            break;
        }

        if(findPos < lineBase)
        {
            findPoint->y = lineIndex;
            findPoint->x = findPos - lineBase + lineMap->fullLen;
            if(findPoint == &dspCurs->begin)
            {
                findPoint = &dspCurs->end;
                findPos += curs->len;
                if(findPos <= lineBase)
                {
                    findPoint->y = lineIndex;
                    findPoint->x = findPos - lineBase + lineMap->fullLen;
                    break;
                }
            }
            else
                break;
        }
    }
}

void _displayCursorToLineCursor(Obj * o, size_t lineIndex, size_t lineSize, SlcCurs * lineCursor)
{
    const size_t beginLine = o->displayCursor.begin.y;
    const size_t endLine = o->displayCursor.end.y;
    const size_t beginPos = o->displayCursor.begin.x;
    const size_t endPos = o->displayCursor.end.x;
    if(lineIndex > beginLine)
    {
        if(lineIndex > endLine)
        {
            lineCursor->pos = lineSize;
            lineCursor->len = 0;
        }
        else if(lineIndex == endLine)
        {
            lineCursor->pos = 0;
            lineCursor->len = endPos;
        }
        else
        {
            lineCursor->pos = 0;
            lineCursor->len = lineSize;
        }
    }
    else if(lineIndex == beginLine)
    {
        if(lineIndex == endLine)
        {
            lineCursor->pos = beginPos;
            lineCursor->len = endPos - beginPos;
        }
        else
        {
            lineCursor->pos = beginPos;
            lineCursor->len = lineSize;
        }
    }
    else
    {
        lineCursor->pos = lineSize;
        lineCursor->len = 0;
    }
}

bool _moveFlagsToTextCursor(Obj * o, uint32_t moveFlags, SlcCurs * textCurs)
{
    bool pageChaged = false;
    uint32_t goal = _getGoalFlagValue(moveFlags);

    if(_getTypeFlagValue(moveFlags) == CURSOR_FLAG_MOVE)
    {
        textCurs->len = 0;

        if(goal == CURSOR_FLAG_PAGE)
             pageChaged = _moveCursorToPageBorder(o, _getBorderFlagValue(moveFlags), textCurs);

        else if(goal == CURSOR_FLAG_LINE)
            _moveCursorToLineBorder(o, _getBorderFlagValue(moveFlags), textCurs);

        else // if(goal == CURSOR_FLAG_CHAR)
            _moveCursorAtChar(o, _getDirectionFlagValue(moveFlags), textCurs);
    }

    else // CURSOR_FLAG_SELECT
    {
        if(goal == CURSOR_FLAG_PAGE)
            _selectAllPage(o, textCurs);

        else if(goal == CURSOR_FLAG_LINE)
            _selectAllLine(o, textCurs);

        else // CURSOR_FLAG_CHAR
            _changeSelectionAtChar(o, _getDirectionFlagValue(moveFlags), textCurs);
    }
    return pageChaged;
}

void _moveCursorToLineBorder(Obj * o, uint32_t borderFlag, SlcCurs * textCurs)
{
    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * const end = lineMap + LINE_AMOUNT;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++)
        if(_posWithinRange(textCurs->pos, lineBase, lineBase + lineMap->fullLen))
            break;

    if(lineMap != end)
    {
        if(borderFlag == CURSOR_FLAG_BEGIN)
            textCurs->pos = lineBase;
        else    // CURSOR_FLAG_END
            textCurs->pos = lineBase + lineMap->payloadLen;
    }
}

bool _moveCursorToPageBorder(Obj * o, uint32_t borderFlag, SlcCurs * textCurs)
{
    if(borderFlag == CURSOR_FLAG_BEGIN)
    {
        textCurs->pos = _calcCurrPageFirstLineBase(o);
        return false;
    }

    if(borderFlag == CURSOR_FLAG_END)
    {
        textCurs->pos = _calcNextPageBase(o) +
                o->pageStruct.lineMapTable[LINE_AMOUNT-1].payloadLen;
        return false;
    }

    if(borderFlag == CURSOR_FLAG_NEXT)
    {
        if(_isCurrPageLast(o))
            return false;
        textCurs->pos = _calcNextPageBase(o)+
                o->pageStruct.lineMapTable[LINE_AMOUNT-1].fullLen;
        _changePageIfNotOnCurrTextPosition(o, textCurs);
    }
    else // CURSOR_FLAG_PREV
    {
        if(_isCurrPageFirst(o))
            return false;
        textCurs->pos = o->pageStruct.base;
        _changePageIfNotOnCurrTextPosition(o, textCurs);
        textCurs->pos = _calcCurrPageFirstLineBase(o);
    }

    DspCurs dspCurs;
    _textCursorToDisplayCursor(o, textCurs, &dspCurs);
    _saveDisplayCursor(o, &dspCurs);

    return true;
}

void _moveCursorAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs)
{
    if(dirFlags == CURSOR_FLAG_LEFT)
    {
        if(textCurs->pos > 0)
        {
            size_t loadLen = textCurs->pos < 10 ? textCurs->pos : 10;
            unicode_t * pchr = _loadLine(o, textCurs->pos-loadLen, loadLen) + loadLen;
            textCurs->pos -= pchr - LPM_TextOperator_prevChar(o->modules->textOperator, pchr);
        }
    }
    else if(dirFlags == CURSOR_FLAG_RIGHT)
    {
        unicode_t * pchr = _loadLine(o, textCurs->pos, 10);
        textCurs->pos += LPM_TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;
    }
    else if(dirFlags == CURSOR_FLAG_UP)
    {
        // TODO: сделать перемещение на строку вверх
    }
    else //if(dirFlags == CURSOR_FLAG_DOWN)
    {
        // TODO: сделать перемещение на строку вниз
    }
}

void _selectAllPage(Obj * o, SlcCurs * textCurs)
{
    textCurs->pos = _calcCurrPageFirstLineBase(o);
    textCurs->len = _calcCurrPageLen(o);
}

void _selectAllLine(Obj * o, SlcCurs * textCurs)
{
    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * const end = lineMap + LINE_AMOUNT;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++)
        if(_posWithinRange(textCurs->pos, lineBase, lineBase + lineMap->fullLen))
            break;

    if(lineMap != end)
    {
        textCurs->pos = lineBase;
        textCurs->len = lineMap->fullLen;
    }
}

void _changeSelectionAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs)
{
    // TODO: сделать изменение выделения на символ и строку
}

uint32_t _getTypeFlagValue(uint32_t flags)
{
    return flags & CURSOR_TYPE_FIELD;
}

uint32_t _getGoalFlagValue(uint32_t flags)
{
    return flags & CURSOR_GOAL_FIELD;
}

uint32_t _getDirectionFlagValue(uint32_t flags)
{
    return flags & CURSOR_DIRECTION_FIELD;
}

uint32_t _getBorderFlagValue(uint32_t flags)
{
    return flags & CURSOR_BORDER_FIELD;
}

void _saveDisplayCursor(Obj * o, const DspCurs * dspCurs)
{
    memcpy(&o->displayCursor, dspCurs, sizeof(DspCurs));
}

void _updateLineChangedFlagsWhenDisplayCursorChanged(Obj * o, DspCurs const * newLineCurs)
{
    // TODO: обновление флагов изменения строк при обновлении курсора дисплея
    // - вычислить симметрическую разницу
    // - обновить флаги
}


void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset)
{
    Unicode_Buf buf =
    {
        .data = o->modules->lineBuffer.data,
        .size = lineMap->payloadLen,
    };
    LPM_TextStorage_read(o->modules->textStorage, lineOffset, &buf);

    unicode_t * pchr = buf.data + lineMap->payloadLen;
    unicode_t * const end = pchr + lineMap->restLen;
    for( ; pchr != end; pchr++)
        *pchr = chrSpace;
}
