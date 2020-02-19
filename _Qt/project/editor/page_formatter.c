#include "page_formatter.h"
#include "lpm_text_storage.h"

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

static void _buildPageFromCurrBase(Obj * o);
static void _rebuildCurrentPage(Obj * o, const SlcCurs * changedTextArea, const SlcCurs * newTextCursor);
static void _updateLineChangedFlagsWhenCursorChanged(Obj * o, const DspCurs * before, const DspCurs * after);

static TextPos _checkTextPos(Obj * o, size_t pos);
static bool _isCurrPageFirst(Obj * o);
static bool _isCurrPageLast(Obj * o);

static size_t _buildLineMap(Obj * o, LineMap * lineMap, uint8_t beginCurs, uint8_t endCurs);

static bool _checkForEndOfText(unicode_t chr);
static bool _checkForEndLine(unicode_t chr);
static bool _checkForDiacritic(unicode_t chr);
static bool _checkForWordDivider(unicode_t chr);

static size_t _buildLineMapWithEndOfText(LineMap * lineMap, size_t currPos, size_t placeCnt);
static size_t _buildLineMapWithEndLine(LineMap * lineMap, const unicode_t * pchr, size_t currPos, size_t placeCnt);
static size_t _buildLineMapWithWordDividerAtLineEnd(LineMap * lineMap, const unicode_t * pchr, size_t currPos);
static size_t _buildLineMapWithVeryLongWord(LineMap * lineMap, const unicode_t * pchr, size_t currPos);
static size_t _buildLineMapWithWordWrap(LineMap * lineMap, size_t lastWordDivPos, size_t placeCntAtLastWordDiv);

static void _loadLine(Obj * o, size_t loadPos);

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

static void _changeDisplayCursorByTextCursor(Obj * o, size_t newTextPosition);

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
          LPM_SelectionCursor * curs )
{
    _setFirstPageInGroup(o, 0);
    _changePageIfNotOnCurrTextPosition(o, curs);
}

void PageFormatter_updatePageByTextChanging
        (PageFormatter * o,
          const LPM_SelectionCursor * changedTextArea,
          const LPM_SelectionCursor * newTextCursor )
{
    _rebuildCurrentPage(o, changedTextArea, newTextCursor);
    _changePageIfNotOnCurrTextPosition(o, newTextCursor);

    // TODO: пересчитать курсор дисплея
    //_changeDisplayCursorByTextCursor(o, newTextCursor);
}


void PageFormatter_updatePageByDisplayCursorChanging
        ( PageFormatter * o,
          LPM_SelectionCursor * textArea,
          uint32_t flags )
{
    // TODO: Переставить курсор
    //_moveDisplayCursor(o, flags);
    // Пересчитать позицию текста
    //_changeTextCursorByDisplayCursor(o, textArea);
    //_changePageIfNotOnCurrTextPosition(o, textArea);
    //                         -----------^ -???
}

void PageFormatter_updateDisplay
        ( PageFormatter * o )
{
    LPM_Point pos = { .x = 0, .y = 0 };
    Unicode_Buf lineBuf = { .data = o->modules->lineBuffer.data, .size = 0 };
    LPM_UnicodeDisplayLineAttr attr = { 0, 0, 0, 0};

    //LPM_UnicodeDisplay_setCursor(o->modules->display, &o->displayCursor);

    const LineMap * lineMap = o->lineMap;
    const LineMap * end     = o->lineMap + LINE_AMOUNT;
    size_t lineOffset = o->currPageBase +
            (_isCurrPageFirst(o) ? 0 : o->prevPageLastLineMap.fullLen);
    size_t lineIndex  = 0;
    for( ; lineMap != end; lineMap++, lineIndex++, pos.y++)
    {
        if(lineIndex == 0)
        {
            attr.lenBeforeSelect = 4;
            attr.lenSelect       = 0;
            attr.lenAfterSelect  = 1;
        }
        else
        {
            attr.lenBeforeSelect = 0;
            attr.lenSelect       = 0;
            attr.lenAfterSelect  = 1;
        }

        if(_readLineChangedFlag(o, lineIndex))
        {
            _formatLineForDisplay(o, lineMap, lineOffset);
            lineBuf.size = lineMap->payloadLen + lineMap->restLen;
            //attr.lenAfterSelect = lineBuf.size;
            LPM_UnicodeDisplay_writeLine(o->modules->display, &lineBuf, &attr);
        }
        lineOffset += lineMap->fullLen;
        attr.index++;
    }
}




void _setFirstPageInGroup(Obj * o, size_t groupIndex)
{
    _switchToFirstPageInGroup(o, groupIndex);
    _buildPageFromCurrBase(o);
}

void _setNextPage(Obj * o)
{
    if(_isCurrPageLast(o))
    {
        _resetAllLineChangedFlags(o);
        return;
    }
    _switchToNextPage(o);
    _buildPageFromCurrBase(o);
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
    o->currPageIndex  = 0;
    o->currGroupIndex = groupIndex;
    o->currPageBase   = o->groupBaseTable[groupIndex];
}

void _switchToNextPage(Obj * o)
{
    o->currPageBase = _calcNextPageBase(o);
    if(_incPageIndex(o))
        o->groupBaseTable[o->currGroupIndex] = o->currPageBase;
}

size_t _calcNextPageBase(Obj * o)
{
    size_t base = o->currPageBase + o->prevPageLastLineMap.fullLen;
    const LineMap * lineMap   = o->lineMap;
    const LineMap * const end = o->lineMap+LINE_AMOUNT-1;
    for( ; lineMap != end; lineMap++)
        base += lineMap->fullLen;
    return base;
}

size_t _calcCurrPageLen(Obj * o)
{
    size_t len = 0;
    const LineMap * lineMap   = o->lineMap;
    const LineMap * const end = o->lineMap+LINE_AMOUNT;
    for( ; lineMap != end; lineMap++)
        len += lineMap->fullLen;
    return len;
}

bool _incPageIndex(Obj * o)
{
    o->currPageIndex++;
    if(o->currGroupIndex < LAST_GROUP)
    {
        if(o->currPageIndex == PAGES_IN_GROUP)
        {
            o->currPageIndex = 0;
            o->currGroupIndex++;
            return true;
        }
    }
    return false;
}

size_t _findGroupWithNearestOffset(Obj * o, size_t pos)
{
    size_t * poffs = o->groupBaseTable + o->currGroupIndex;
    size_t * const pend = o->groupBaseTable;
    size_t groupIndex = o->currGroupIndex;
    for( ; poffs != pend; poffs--, groupIndex--)
        if(*poffs < pos)
            break;
    return groupIndex;
}

void _buildPageFromCurrBase(Obj * o)
{
    _setAllLineChangedFlags(o);
    o->lastPageReached = false;

    size_t lineOffset = 0;
    if(!_isCurrPageFirst(o))
    {
        _loadLine(o, o->currPageBase);
        lineOffset = _buildLineMap(o, &o->prevPageLastLineMap, 255, 255);
    }

    LineMap * lineMap   = o->lineMap;
    LineMap * const end = o->lineMap + LINE_AMOUNT;
    for( ; lineMap != end; lineMap++)
    {
        _loadLine(o, o->currPageBase + lineOffset);
        lineOffset += _buildLineMap(o, lineMap, 255, 255);
    }
}

void _rebuildCurrentPage(Obj * o, const SlcCurs * changedTextArea, const SlcCurs * newTextCursor)
{
    _resetAllLineChangedFlags(o);
    o->lastPageReached = false;

    size_t lineEndPosBeforeLineBuild = 0;
    size_t lineEndPosAfterLineBuild  = 0;
    size_t lineOffset = 0;

    if(!_isCurrPageFirst(o))
    {
        _loadLine(o, o->currPageBase);
        lineEndPosBeforeLineBuild = o->currPageBase + o->prevPageLastLineMap.fullLen;
        lineOffset += _buildLineMap(o, &o->prevPageLastLineMap, 255, 255);
        lineEndPosAfterLineBuild  = o->currPageBase + o->prevPageLastLineMap.fullLen;
    }

    // Скопировать курсор дисплея. В цикле курсор дисплея и куросор знакомест
    //  пересчитаются (в функции _buildLineMap)
    DspCurs notModifiedCursor;
    memcpy(&notModifiedCursor, &o->displayCursor, sizeof(DspCurs));
    size_t lineIndex = 0;
    LineMap * lineMap = o->lineMap;
    LineMap * end     = o->lineMap + LINE_AMOUNT;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {
        if(lineEndPosBeforeLineBuild != lineEndPosAfterLineBuild)
            _setLineChangedFlag(o, lineIndex);

        _loadLine(o, o->currPageBase + lineOffset);

        lineEndPosBeforeLineBuild = o->currPageBase + lineOffset + lineMap->fullLen;
        size_t lineOffsetInc = _buildLineMap(o, lineMap, 255, 255);
        lineEndPosAfterLineBuild = o->currPageBase + lineOffset + lineMap->fullLen;

        if(lineEndPosBeforeLineBuild != lineEndPosAfterLineBuild)
            _setLineChangedFlag(o, lineIndex);

        if(_changedTextCrossesLine( changedTextArea,
                                    o->currPageBase + lineOffset,
                                    lineMap->fullLen ))
            _setLineChangedFlag(o, lineIndex);

        lineOffset += lineOffsetInc;
    }

    _updateLineChangedFlagsWhenCursorChanged(o, &notModifiedCursor, &o->displayCursor);

    // Вычислить объединение по ИЛИ обновленного и сохраненного курсоров
    //  знакомест и на его основе вычислить флаги обновления строк
}

void _updateLineChangedFlagsWhenCursorChanged(Obj * o, const DspCurs * before, const DspCurs * after)
{}

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
        return pos >= o->currPageBase + o->prevPageLastLineMap.fullLen ?
                    TEXT_POS_ON_CURRENT_PAGE :
                    TEXT_POS_BEFORE_CURRENT_PAGE;
    }

    size_t currPageBegin = o->currPageBase + o->prevPageLastLineMap.fullLen;
    size_t currPageEnd   = currPageBegin + _calcCurrPageLen(o);

    if(pos < currPageBegin)
        return TEXT_POS_BEFORE_CURRENT_PAGE;

    if(pos >= currPageEnd)
        return TEXT_POS_AFTER_CURRENT_PAGE;

    return TEXT_POS_ON_CURRENT_PAGE;
}

bool _isCurrPageFirst(Obj * o)
{
    return (o->currPageIndex == 0) && (o->currGroupIndex == 0);
}

bool _isCurrPageLast(Obj * o)
{
    return o->lastPageReached;
}

size_t _buildLineMap(Obj * o, LineMap * lineMap, uint8_t beginCurs, uint8_t endCurs)
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
            return _buildLineMapWithEndOfText(lineMap, currPos, placeCnt);
        }

        if(_checkForEndLine(*pchr))
            return _buildLineMapWithEndLine(lineMap, pchr, currPos, placeCnt);

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

        if(lastNoDiacriticPos == beginCurs)
        {
            o->displayCursor.begin.x   = placeCnt;
            o->displayCursor.begin.y   = lineMap - o->lineMap;
            o->signPlaceCursor.begin.x = lastNoDiacriticPos;
            o->signPlaceCursor.begin.y = lineMap - o->lineMap;
        }
        if(lastNoDiacriticPos == endCurs)
        {
            o->displayCursor.end.x   = placeCnt;
            o->displayCursor.end.y   = lineMap - o->lineMap;
            o->signPlaceCursor.end.x = lastNoDiacriticPos;
            o->signPlaceCursor.end.y = lineMap - o->lineMap;
        }

        placeCnt++;
        lastNoDiacriticPos = currPos;
    }

    if(_checkForWordDivider(*pchr))
        return _buildLineMapWithWordDividerAtLineEnd(lineMap, pchr, currPos);

    if(lastWordDivPos == wordDividersNotFoundValue)
        return _buildLineMapWithVeryLongWord(lineMap, pchr, currPos);

    return _buildLineMapWithWordWrap(lineMap, lastWordDivPos, placeCntAtLastWordDiv);
}

bool _checkForEndOfText(unicode_t chr)
{
    return chr == chrEndOfText;
}

bool _checkForEndLine(unicode_t chr)
{
    return (chr == chrCr) || (chr == chrLf);
}

bool _checkForDiacritic(unicode_t chr)
{
    return Unicode_isChrDiacritic(chr);
}

bool _checkForWordDivider(unicode_t chr)
{
    return (chr == chrSpace) || (chr == chrCr) || (chr == chrLf);
}

size_t _buildLineMapWithEndOfText(LineMap * lineMap, size_t currPos, size_t placeCnt)
{
    lineMap->fullLen    = currPos;
    lineMap->payloadLen = currPos;
    lineMap->restLen    = CHAR_AMOUNT - placeCnt;
    return lineMap->fullLen;
}

size_t _buildLineMapWithEndLine(LineMap * lineMap, const unicode_t * pchr, size_t currPos, size_t placeCnt)
{
    lineMap->payloadLen = currPos;
    lineMap->restLen = CHAR_AMOUNT - placeCnt;
    lineMap->fullLen = currPos+1;
    if((pchr[0] == chrCr) && (pchr[1] == chrLf))
        lineMap->fullLen++;
    return lineMap->fullLen;
}

size_t _buildLineMapWithWordDividerAtLineEnd(LineMap * lineMap, const unicode_t * pchr, size_t currPos)
{
    lineMap->payloadLen = currPos;
    lineMap->restLen = 0;
    lineMap->fullLen = currPos;

    if(*pchr == chrSpace)
        lineMap->fullLen++;
    ++pchr;

    if(*pchr == chrCr)
        lineMap->fullLen++;
    ++pchr;

    if(*pchr == chrLf)
        lineMap->fullLen++;

    return lineMap->fullLen;
}

size_t _buildLineMapWithVeryLongWord(LineMap * lineMap, const unicode_t * pchr, size_t currPos)
{
    if(_checkForDiacritic(*pchr))
        currPos++;
    lineMap->payloadLen = currPos;
    lineMap->restLen = 0;
    lineMap->fullLen = currPos;
    return lineMap->fullLen;
}

size_t _buildLineMapWithWordWrap(LineMap * lineMap, size_t lastWordDivPos, size_t placeCntAtLastWordDiv)
{
    lineMap->payloadLen = lastWordDivPos;
    lineMap->restLen = CHAR_AMOUNT - placeCntAtLastWordDiv;
    lineMap->fullLen = lastWordDivPos+1;
    return lineMap->fullLen;
}

void _loadLine(Obj * o, size_t loadPos)
{
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        o->modules->lineBuffer.size
    };
    size_t fullSize = o->modules->lineBuffer.size;
    LPM_TextStorage_read(o->modules->textStorage, loadPos, &buf);
    memset(buf.data + buf.size, 0, (fullSize - buf.size)*sizeof(unicode_t));
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

void _changeDisplayCursorByTextCursor(Obj * o, size_t newTextPosition)
{
    DspCurs newCursor = {{0,0},{0,0}};
    const LineMap * lineMap = o->lineMap;
    const LineMap * const end = lineMap + LINE_AMOUNT;
    size_t lineOffset = o->prevPageLastLineMap.fullLen + o->currPageBase;
    for( ; lineMap != end; lineMap++)
    {
        //if(lineMap == o->lineMap+10)
            //test_beep(lineMap-o->lineMap, lineMap->fullLen);
        if( (newTextPosition >= lineOffset) &&
            (newTextPosition < lineOffset + lineMap->fullLen) )
            break;

        lineOffset += lineMap->fullLen;
        newCursor.begin.y++;
        newCursor.end.y++;
    }

    if(lineMap == end)
        return;

    newCursor.begin.x = newTextPosition-lineOffset;
    newCursor.end.x = newTextPosition-lineOffset;

    memcpy(&o->displayCursor, &newCursor, sizeof(DspCurs));
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
