#include "page_formatter.h"
#include "lpm_text_storage.h"
#include "lpm_text_operator.h"

#include <string.h>

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

static void _setFirstPageInGroup(Obj * o, size_t groupIndex);
static void _setNextPage(Obj * o);
static void _changePageIfNotOnCurrTextPosition(Obj * o, const SlcCurs * pos);

static void _switchToFirstPageInGroup(Obj * o, size_t groupIndex);
static void _switchToNextPage(Obj * o);
static size_t _calcNextPageBase(Obj * o);
static size_t _calcCurrPageLen(Obj * o);
static bool _incPageIndex(Obj * o);
static size_t _findGroupWithNearestOffset(Obj * o, size_t pos);

static void _updateLinesMap(Obj * o);

static TextPos _checkTextPos(Obj * o, size_t pos);
static bool _isCurrPageFirst(Obj * o);
static bool _isCurrPageLast(Obj * o);

static size_t _updateLineMap(Obj * o, LineMap * lineMap, size_t lineBase);

static void _makePrevLineMapForFirstPage(LineMap * lineMap);

static unicode_t * _loadLine(Obj * o, size_t loadPos);

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

static void _calcLinesCursorWhenTextChanged(Obj * o, const SlcCurs * curs, DspCurs * newLineCurs);
static void _calcLinesCursorWhenCursorMoved(Obj * o, uint32_t moveFlags, DspCurs * newLineCurs);
static void _updateLineChangedFlagsWhenLineCursorChanged(Obj * o, DspCurs const * newLineCurs);

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
    _setFirstPageInGroup(o, 0);
    _changePageIfNotOnCurrTextPosition(o, curs);
}

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs  )
{
    _updateLinesMap(o);
    _changePageIfNotOnCurrTextPosition(o, curs);
    DspCurs newLineCurs;
    _calcLinesCursorWhenTextChanged(o, curs, &newLineCurs);
    _updateLineChangedFlagsWhenLineCursorChanged(o, &newLineCurs);
}


void PageFormatter_updatePageWhenCursorMoved
    ( PageFormatter * o,
      uint32_t moveFlags )
{
    DspCurs newLineCurs;
    _calcLinesCursorWhenCursorMoved(o, moveFlags, &newLineCurs);
    _updateLineChangedFlagsWhenLineCursorChanged(o, &newLineCurs);
}

void PageFormatter_updateDisplay
        ( PageFormatter * o )
{
    LPM_Point pos = { .x = 0, .y = 0 };
    Unicode_Buf lineBuf = { .data = o->modules->lineBuffer.data, .size = 0 };
    LPM_SelectionCursor curs;

    //LPM_UnicodeDisplay_setCursor(o->modules->display, &o->displayCursor);

    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * end     = o->pageStruct.lineMapTable + LINE_AMOUNT;
    size_t lineOffset = o->pageStruct.base +
            (_isCurrPageFirst(o) ? 0 : o->pageStruct.prevLastLine.fullLen);
    size_t lineIndex  = 0;
    for( ; lineMap != end; lineMap++, lineIndex++, pos.y++)
    {
        if(lineIndex == 0)
        {
            curs.pos = 10;
            curs.len = 12;
        }
        else
        {
            curs.pos = lineBuf.size;
            curs.len = 0;
        }

        if(_readLineChangedFlag(o, lineIndex))
        {
            _formatLineForDisplay(o, lineMap, lineOffset);
            lineBuf.size = lineMap->payloadLen + lineMap->restLen;
            //attr.lenAfterSelect = lineBuf.size;
            LPM_UnicodeDisplay_writeLine(o->modules->display, lineIndex, &lineBuf, &curs);
        }
        lineOffset += lineMap->fullLen;
    }
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

void _changePageIfNotOnCurrTextPosition(Obj * o, const SlcCurs * pos)
{
    TextPos tp = _checkTextPos(o, pos->pos);

    if(tp == TEXT_POS_ON_CURRENT_PAGE)
    {
        return;
    }

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
    size_t base = o->pageStruct.base + o->pageStruct.prevLastLine.fullLen;
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
    for( ; lineMap != end; lineMap++)
        lineBase += _updateLineMap(o, lineMap, lineBase);

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
        return pos >= o->pageStruct.base+ o->pageStruct.prevLastLine.fullLen ?
                    TEXT_POS_ON_CURRENT_PAGE :
                    TEXT_POS_BEFORE_CURRENT_PAGE;
    }

    size_t currPageBegin = o->pageStruct.base + o->pageStruct.prevLastLine.fullLen;
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
    const unicode_t * const begin = _loadLine(o, lineBase);

    LPM_TextLineMap textLineMap;
    if(LPM_TextOperator_analizeLine( o->modules->textOperator,
                                     begin,
                                     CHAR_AMOUNT,
                                     &textLineMap) )
        o->pageStruct.lastPageReached = true;

    lineMap->fullLen    = (uint8_t)(textLineMap.nextLine    - begin);
    lineMap->payloadLen = (uint8_t)(textLineMap.printBorder - begin);
    lineMap->restLen    = CHAR_AMOUNT - textLineMap.lenInChr;
    // TODO: считать CRC и сравнивать с предыдущим
    //lineMap->crc = _calcCrc();

    return lineMap->fullLen;
}

void _makePrevLineMapForFirstPage(LineMap * lineMap)
{
    memset(lineMap, 0, sizeof(LineMap));
}

unicode_t * _loadLine(Obj * o, size_t loadPos)
{
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        o->modules->lineBuffer.size
    };
    size_t fullSize = o->modules->lineBuffer.size;
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

void _calcLinesCursorWhenTextChanged(Obj * o, const SlcCurs * curs, DspCurs * newLineCurs)
{
    // TODO: расчет курсора строк по курсору текста
}

void _calcLinesCursorWhenCursorMoved(Obj * o, uint32_t moveFlags, DspCurs * newLineCurs)
{
    // TODO: расчет курсора строк по курсору дисплея, флагам и курсору строк в предыдущий момент
}

void _updateLineChangedFlagsWhenLineCursorChanged(Obj * o, DspCurs const * newLineCurs)
{
    // TODO: вычислить симметрическую разницу
    // обновить флаги
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

#if 0

size_t _buildLineMap(Obj * o, LineMap * map, uint8_t bTxtSlc, uint8_t eTxtSlc)
{
    const size_t maxPlaceCntValue = CHAR_AMOUNT;
    const size_t wordDividersNotFoundValue = o->modules->lineBuffer.size;

    size_t currPos = 0;
    size_t placeCnt = 0;
    size_t lastWordDivPos = wordDividersNotFoundValue;
    size_t placeCntAtLastWordDiv = 0;
    size_t lastNoDiacriticPos = 0;

    const unicode_t * pchr = o->modules->lineBuffer.data;
    const unicode_t * pend = pchr + o->modules->lineBuffer.size;
    for( ; pchr != pend; pchr++, currPos++ )
    {
        if(_checkForEndOfText(*pchr))
        {
            o->lastPageReached = true;
            return _buildLineMapWithEndOfText(map, currPos, placeCnt);
        }

        if(_checkForEndLine(*pchr))
            return _buildLineMapWithEndLine(map, pchr, currPos, placeCnt);

        if(placeCnt == maxPlaceCntValue)
            break;

        if(_checkForDiacritic(*pchr))
            continue;

        if(_checkForWordDivider(*pchr))
        {
            // Запоминаем позицию символа-разделителя слов, а так же значения
            //  счетчика занятых знакомест в этот момент
            lastWordDivPos = currPos;
            placeCntAtLastWordDiv = placeCnt;
        }

        // Здесь добавить сравнение с положением курсора текста, переданного в
        //  функцию, и если нужно, обновить курсор дисплея и курсор знакомест

        if(lastNoDiacriticPos == bTxtSlc)
        {
            o->displayCursor.begin.x   = placeCnt;
            o->displayCursor.begin.y   = map - o->lineMap;
        }
        if(lastNoDiacriticPos == eTxtSlc)
        {
            o->displayCursor.end.x   = placeCnt;
            o->displayCursor.end.y   = map - o->lineMap;
        }

        placeCnt++;
        lastNoDiacriticPos = currPos;
    }

    if(_checkForWordDivider(*pchr))
        return _buildLineMapWithWordDividerAtLineEnd(map, pchr, currPos);

    if(lastWordDivPos == wordDividersNotFoundValue)
        return _buildLineMapWithVeryLongWord(map, pchr, currPos);

    return _buildLineMapWithWordWrap(map, lastWordDivPos, placeCntAtLastWordDiv);
}

#endif
