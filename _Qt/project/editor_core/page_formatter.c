#include "page_formatter.h"
#include "text_storage.h"
#include "text_operator.h"
#include "crc16_table.h"
#include "editor_flags.h"
#include "line_buffer_support.h"

#include <string.h>

typedef PageFormatter Obj;
typedef LPM_DisplayCursor DspCurs;
typedef LPM_SelectionCursor SlcCurs;

typedef struct PageAttr
{
    size_t begin;
    size_t end;
    bool isFirst;
    bool isLast;
} PageAttr;

typedef enum PageStatus
{
    PAGE_CURR,
    PAGE_NEXT,
    PAGE_PREV,
} PageStatus;

static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrSpace = 0x0020;

static void _updatePageByTextCursor(Obj * o, const SlcCurs * curs);

static void _setFirstPageInGroup(Obj * o, size_t groupIndex);
static void _setNextPage(Obj * o);
static void _setFirstPageInNearestGroup(Obj * o, size_t pos);
static bool _changePageIfNotOnCurrTextPosition(Obj * o, size_t pos);

static void _fillCurrPageAttr(Obj * o, PageAttr * attr);
static bool _changePageIfNotOnCurrTextPositionWhenPageFirst(Obj * o, size_t pos, const PageAttr * attr);
static bool _changePageIfNotOnCurrTextPositionWhenPageLast(Obj * o, size_t pos, const PageAttr * attr);
static bool _changePageIfNotOnCurrTextPositionWhenPageCommon(Obj * o, size_t pos, const PageAttr * attr);

static void _switchToFirstPageInGroup(Obj * o, size_t groupIndex);
static void _switchToNextPage(Obj * o);
static size_t _calcNextPageBase(Obj * o);
static size_t _calcCurrPageLen(Obj * o);
static size_t _calcCurrPageFirstLineBase(Obj * o);
static size_t _calcLineBase(Obj * o, size_t lineIndex);
static bool _incPageIndex(Obj * o);
static size_t _findGroupWithNearestOffset(Obj * o, size_t pos);

static void _updateLinesMap(Obj * o);

static bool _isCurrPageFirst(Obj * o);
static bool _isCurrPageLast(Obj * o);
static bool _isCurrPageEmpty(Obj * o);
static bool _isPrevLastLineWithEndLine(Obj * o);

static size_t _updateLineMap(Obj * o, LineMap * lineMap, size_t lineBase);

static void _copyLineMap(LineMap * dst, const LineMap * src);
static bool _lineChanged(const LineMap * before, const LineMap * after);
static uint16_t _calcLineCrc(Obj * o, LineMap * lineMap);
static void _makePrevLineMapForFirstPage(LineMap * lineMap);

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
static LineMap * _findLineAtCursorPos(Obj * o, const SlcCurs * textCurs, size_t * pLineBase);

static void _updateDisplayCursorByTextCursor(Obj * o, const SlcCurs * curs);
static size_t _calcDisplayCursorByLineMapAnalizing(Obj * o, const SlcCurs * txtCurs, DspCurs * dspCurs);
static void _setDisplayCursorValueForEmptyPage(DspCurs * curs);
static void _setDisplayCursorInvalidValue(Obj * o);
static bool _isDisplayCursorPointInvalid(Obj * o, const LPM_Point * point);
static void _validateDisplayCursorPointIfInvalid(Obj * o, size_t lineIndex, LPM_Point * point);


static void _displayLine(Obj * o, size_t lineIndex, size_t lineOffset);
static void _readDisplayedLineToBuffer(Obj * o, const LineMap * lineMap, size_t lineOffset, Unicode_Buf * lineBuf);
static size_t _calcPositionOfDisplayXvalue(Obj * o, size_t x);

static void _displayCursorToLineCursor(Obj * o, size_t lineIndex, SlcCurs * lineCursor);

static PageStatus _moveFlagsToDisplayCursor(Obj * o, uint32_t moveFlags, DspCurs * dspCurs);
static void _moveCursorToLineBorder(Obj * o, uint32_t borderFlag, DspCurs * dspCurs);
static PageStatus _moveCursorToPageBorder(Obj * o, uint32_t borderFlag, DspCurs * dspCurs);
static PageStatus _moveCursorAtChar(Obj * o, uint32_t dirFlags, DspCurs * dspCurs);

static void _selectAllPage(Obj * o, DspCurs * dspCurs);
static void _selectAllLine(Obj * o, DspCurs * dspCurs);
static void _changeSelectionAtChar(Obj * o, uint32_t dirFlags, DspCurs * dspCurs);
static void _normalizeSelectionDirAndSwapDspPointsIfNeed(Obj * o, uint32_t dirFlags, DspCurs * dspCurs);

static bool _textSelected(const DspCurs * dspCurs);
static bool _incDspXPos(Obj * o, LPM_Point * dspPoint);
static bool _decDspXPos(Obj * o, LPM_Point * dspPoint);
static bool _incDspYPos(Obj * o, LPM_Point * dspPoint);
static bool _decDspYPos(Obj * o, LPM_Point * dspPoint);
static void _setDspPosToLineBegin(Obj * o, LPM_Point * dspPoint);
static void _setDspPosToLineEnd(Obj * o, LPM_Point * dspPoint);
static void _setDspPosToPageBegin(Obj * o, LPM_Point * dspPoint);
static void _setDspPosToPageEnd(Obj * o, LPM_Point * dspPoint);
static void _copyDspPoint(LPM_Point * dst, const LPM_Point * src);
static void _swapDspPoints(DspCurs * dspCurs);

static void _changePageByStatus(Obj * o, PageStatus ps);

static void _calcTextCursorByDisplayCursorWhenPageChangedAndChangPage(Obj * o, uint32_t moveFlags, SlcCurs * textCurs);

static void _displayCursorToTextCursor(Obj * o, SlcCurs * textCursor);

static bool _dspXPosIsAfterLinePayloadPart(Obj * o, const LineMap * lm, size_t xPos);
static size_t _calcTextPosByDisplayXPos(Obj * o, size_t lineBase, const LineMap * lm, size_t xPos);
static size_t _calcSpaceAmount(Obj * o, const LineMap * lm, size_t xPos);

static void _textCursorToDisplayCursor(Obj * o, DspCurs * dspCurs, const SlcCurs * txtCurs);
static void _setInvalidDisplayCursorValue(Obj * o, DspCurs * dspCurs);
static size_t _calcDisplayCursorYValues(Obj * o, DspCurs * dspCurs, const SlcCurs * txtCurs);
static void _validateDisplayPointIfInvalid(Obj * o, LPM_Point * point, size_t firstEmptyLineIndex);
static void _calcDisplayPointXValue(Obj * o, LPM_Point * point, size_t txtPos);

static size_t _movePosLeftAtChar(Obj * o, size_t pos);
static size_t _movePosRightAtChar(Obj * o, size_t pos);
static size_t _movePosUpAtChar(Obj * o, size_t textPos, const LPM_Point * dspPoint);
static size_t _movePosDownAtChar(Obj * o, size_t textPos, const LPM_Point * dspPoint);

static uint32_t _getTypeFlagValue(uint32_t flags);
static uint32_t _getGoalFlagValue(uint32_t flags);
static uint32_t _getDirectionFlagValue(uint32_t flags);
static uint32_t _getBorderFlagValue(uint32_t flags);

static void _saveTextCursor(Obj * o, const SlcCurs * textCurs);
static void _updateLineChangedFlagsWhenTextCursorChanged(Obj * o, const SlcCurs * newTextCursor);
static void _updateLineChangedFlagsByDisplayCursor(Obj * o, const DspCurs * dspCurs);
static void _displayCursorToLinearCursor(Obj * o, SlcCurs * slcCurs, const DspCurs * dspCurs);
static void _setLineChangedFlagsBySymmetricDifferencePart(Obj * o, const SlcCurs * symDifPart);
static void _copyDisplayCursor(DspCurs * dst, const DspCurs * src);

static void _calcSymmetricDifference
        ( const SlcCurs * src1,
          const SlcCurs * src2,
          SlcCurs * dst1,
          SlcCurs * dst2 );
static void _normalizeTextCursorAndFillBeginEnd(const SlcCurs * curs, size_t * begin, size_t * end);
static void _updateLineChangedFlagsByTextCursorChanging
        (Obj * o, const SlcCurs * symDiff1, const SlcCurs * symmDiff2);

static void _displayLine(Obj * o, size_t lineIndex, size_t lineOffset);
static void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset);

static size_t _calcPageLastLineIndex(Obj * o);

void test_beep();

void PageFormatter_init
        ( PageFormatter * o,
          const Modules * modules,
          LPM_UnicodeDisplay * display,
          const LPM_EditorPageParams * pageParams )
{
    memset(o, 0, sizeof(PageFormatter));
    o->modules = modules;
    o->display = display;
    o->pageParams = pageParams;
    o->pageStruct.lineMapTable = modules->lineMapTable;
    o->pageNavi.groupBaseTable = modules->pageGroupBaseTable;
    memset(o->pageNavi.groupBaseTable, 0, sizeof(size_t) * o->pageParams->pageGroupAmount);
    memset(o->pageStruct.lineMapTable, 0, sizeof(LineMap) * o->pageParams->lineAmount);
}

void PageFormatter_startWithPageAtTextPosition
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs )
{
    o->spaceAmount = 0;
    _setFirstPageInGroup(o, 0);
    _changePageIfNotOnCurrTextPosition(o, curs->pos);
    _updateDisplayCursorByTextCursor(o, curs);
    _saveTextCursor(o, curs);
}

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs )
{
    o->spaceAmount = 0;
    _resetAllLineChangedFlags(o);
    _updateLinesMap(o);

    DspCurs dspCursor;
    _copyDisplayCursor(&dspCursor, &o->displayCursor);

    if(!_changePageIfNotOnCurrTextPosition(o, curs->pos))
    {
        _textCursorToDisplayCursor(o, &dspCursor, curs);
        _updateLineChangedFlagsByDisplayCursor(o, &dspCursor);
    }
    else
    {
        _textCursorToDisplayCursor(o, &dspCursor, curs);
    }

    _copyDisplayCursor(&o->displayCursor, &dspCursor);

    //_updatePageByTextCursor(o, curs);
}


void PageFormatter_updatePageWhenCursorMoved
    ( PageFormatter * o,
      uint32_t moveFlags,
      LPM_SelectionCursor * textCurs )
{
    _resetAllLineChangedFlags(o);

    DspCurs dspCursor;
    _copyDisplayCursor(&dspCursor, &o->displayCursor);

    PageStatus pageStatus = _moveFlagsToDisplayCursor(o, moveFlags, &dspCursor);

    _changePageByStatus(o, pageStatus);

    if(pageStatus == PAGE_CURR)
        _updateLineChangedFlagsByDisplayCursor(o, &dspCursor);
    _copyDisplayCursor(&o->displayCursor, &dspCursor);

    _displayCursorToTextCursor(o, textCurs);
}

void PageFormatter_updateWholePage(PageFormatter * o)
{
    o->spaceAmount = 0;
    _setAllLineChangedFlags(o);
    PageFormatter_updateDisplay(o);
}

void PageFormatter_updateDisplay
        ( PageFormatter * o )
{
//    Unicode_Buf lineBuf = { o->modules->lineBuffer.data, 0 };
//    LPM_SelectionCursor lineCursor;

    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * end     = lineMap + o->pageParams->lineAmount;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    size_t lineIndex  = 0;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {
        if(_readLineChangedFlag(o, lineIndex))
        {
            _displayLine(o, lineIndex, lineBase);
//            lineBuf.size = lineMap->payloadLen + lineMap->restLen;
//            _displayCursorToLineCursor(o, lineIndex, lineBuf.size, &lineCursor);
//            _formatLineForDisplay(o, lineMap, lineBase);
//            _displayLine(o, lineIndex);
//            LPM_UnicodeDisplay_writeLine( o->display,
//                                          lineIndex,
//                                          &lineBuf,
//                                          &lineCursor);
        }
        lineBase += lineMap->fullLen;
    }
}

size_t PageFormatter_getCurrLinePos(PageFormatter * o)
{
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    size_t currIndex = o->displayCursor.begin.y;

    if(currIndex < o->pageParams->lineAmount)
    {
        size_t lineIndex  = 0;
        const LineMap * lineMap = o->pageStruct.lineMapTable;
        const LineMap * end     = lineMap + o->pageParams->lineAmount;
        for( ; lineMap != end; lineMap++, lineIndex++)
        {
            if(lineIndex == currIndex)
                break;
            lineBase += lineMap->fullLen;
        }
    }
    return lineBase;
}

size_t PageFormatter_getCurrLineLen(PageFormatter * o)
{
    size_t currIndex = o->displayCursor.begin.y;
    if(currIndex < o->pageParams->lineAmount)
        return o->pageStruct.lineMapTable[currIndex].fullLen;
    return 0;
}

size_t PageFormatter_getCurrPagePos(PageFormatter * o)
{
    return _calcCurrPageFirstLineBase(o);
}

size_t PageFormatter_getCurrPageLen(PageFormatter * o)
{
    return _calcCurrPageLen(o);
}

void _updatePageByTextCursor(Obj * o, const SlcCurs * curs)
{
    if(!_changePageIfNotOnCurrTextPosition(o, curs->pos))
    {
        _updateDisplayCursorByTextCursor(o, curs);
        _updateLineChangedFlagsWhenTextCursorChanged(o, curs);
    }
    else
    {
        _updateDisplayCursorByTextCursor(o, curs);
    }
    _saveTextCursor(o, curs);
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

void _setFirstPageInNearestGroup(Obj * o, size_t pos)
{
    size_t groupIndex = _findGroupWithNearestOffset(o, pos);
    _setFirstPageInGroup(o, groupIndex);
}

bool _changePageIfNotOnCurrTextPosition(Obj * o, size_t pos)
{
    size_t initGroupIndex = o->pageNavi.currGroupIndex;
    size_t initPageIndex  = o->pageNavi.currPageIndex;

    size_t itCount;
    for(itCount = 0; itCount < 1024; itCount++)
    {
        PageAttr pa;
        _fillCurrPageAttr(o, &pa);

        bool textPosIsOnCurrPage;

        if(pa.isFirst && pa.isLast)
            textPosIsOnCurrPage = true;
        else if(pa.isFirst)
            textPosIsOnCurrPage = _changePageIfNotOnCurrTextPositionWhenPageFirst(o, pos, &pa);
        else if(pa.isLast)
            textPosIsOnCurrPage = _changePageIfNotOnCurrTextPositionWhenPageLast(o, pos, &pa);
        else
            textPosIsOnCurrPage = _changePageIfNotOnCurrTextPositionWhenPageCommon(o, pos, &pa);


        if(textPosIsOnCurrPage)
            break;
    }
    if(itCount == 1024)
        test_beep();

    return initGroupIndex != o->pageNavi.currGroupIndex ||
            initPageIndex != o->pageNavi.currPageIndex;
}

void _fillCurrPageAttr(Obj * o, PageAttr * attr)
{
    attr->begin   = _calcCurrPageFirstLineBase(o);
    attr->end     = attr->begin + _calcCurrPageLen(o);
    attr->isFirst = _isCurrPageFirst(o);
    attr->isLast  = _isCurrPageLast(o);
}

bool _changePageIfNotOnCurrTextPositionWhenPageFirst(Obj * o, size_t pos, const PageAttr * attr)
{
    if(pos < attr->end)
        return true;

    // Защита от зацикливания: если достигли последней страницы, то нет смысла
    //  продолжать переключаться на следующую страницу
    _setNextPage(o);
    return _isCurrPageLast(o);
}

bool _changePageIfNotOnCurrTextPositionWhenPageLast(Obj * o, size_t pos, const PageAttr * attr)
{
    if(pos >= attr->begin)
    {
        // Особый случай: если последняя страница пустая, то ее конец совпадает
        //  с началом, поэтому курсор текста будет всегда ВНЕ пустой страницы.
        //  Значит, по обычным правилам - страницу нужно ВСЕГДА переключать на
        //  предыдущую. А в том случае, если предыдущая страница оканчивалась
        //  символом перевода строки, нам нужно ОСТАТЬСЯ на последней странице.
        //  Соответственно - нужно этот случай учесть и переключать страницу
        //  только тогда, когда нет перевода строки.
        if(_isCurrPageEmpty(o) && !_isPrevLastLineWithEndLine(o))
        {
            _setFirstPageInNearestGroup(o, pos);
            return false;
        }
        return true;
    }

    _setFirstPageInNearestGroup(o, pos);
    return false;
}

bool _changePageIfNotOnCurrTextPositionWhenPageCommon(Obj * o, size_t pos, const PageAttr * attr)
{
    if(pos < attr->begin)
    {
        _setFirstPageInNearestGroup(o, pos);
        return false;
    }

    if(pos >= attr->end)
    {
        // Защита от зацикливания: если достигли последней страницы, то нет смысла
        //  продолжать переключаться на следующую страницу
        _setNextPage(o);
        return _isCurrPageLast(o);
    }

    return true;
}

void _switchToFirstPageInGroup(Obj * o, size_t groupIndex)
{
    o->pageNavi.currPageIndex  = 0;
    o->pageNavi.currGroupIndex = groupIndex;
    o->pageStruct.base         = o->pageNavi.groupBaseTable[groupIndex];
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
    const LineMap * const end = o->pageStruct.lineMapTable+o->pageParams->lineAmount-1;
    for( ; lineMap != end; lineMap++)
        base += lineMap->fullLen;
    return base;
}

size_t _calcCurrPageLen(Obj * o)
{
    size_t len = 0;
    const LineMap * lineMap   = o->pageStruct.lineMapTable;
    const LineMap * const end = o->pageStruct.lineMapTable+o->pageParams->lineAmount;
    for( ; lineMap != end; lineMap++)
        len += lineMap->fullLen;
    return len;
}

size_t _calcCurrPageFirstLineBase(Obj * o)
{
    return o->pageStruct.base + o->pageStruct.prevLastLine.fullLen;
}

size_t _calcLineBase(Obj * o, size_t lineIndex)
{
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    const LineMap * lm = o->pageStruct.lineMapTable;
    const LineMap * const end = lm + lineIndex;
    for( ; lm != end; lm++)
        lineBase += lm->fullLen;
    return lineBase;
}

bool _incPageIndex(Obj * o)
{
    o->pageNavi.currPageIndex++;
    if(o->pageNavi.currGroupIndex < (o->pageParams->pageGroupAmount-1u))
    {
        if(o->pageNavi.currPageIndex == o->pageParams->pageInGroupAmount)
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
    {
        LineMap lastLine;
        _updateLineMap(o, &lastLine, *poffs);
        if(*poffs + lastLine.fullLen < pos)
            break;
    }
    return groupIndex;
}

void _updateLinesMap(Obj * o)
{    
    o->pageStruct.lastPageReached = false;

    size_t lineBase = o->pageStruct.base;
    if(!_isCurrPageFirst(o))
    {
        _updateLineMap(o, &o->pageStruct.prevLastLine, lineBase);
        lineBase += o->pageStruct.prevLastLine.fullLen;
    }
    else
        _makePrevLineMapForFirstPage(&o->pageStruct.prevLastLine);

    LineMap * lineMap   = o->pageStruct.lineMapTable;
    LineMap * const end = o->pageStruct.lineMapTable + o->pageParams->lineAmount;
    LineMap copy;
    size_t lineIndex = 0;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {
        _copyLineMap(&copy, lineMap);
        if(_updateLineMap(o, lineMap, lineBase))
            o->pageStruct.lastPageReached = true;
        lineBase += lineMap->fullLen;
        if(_lineChanged(lineMap, &copy))
            _setLineChangedFlag(o, lineIndex);
    }
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

bool _isCurrPageEmpty(Obj * o)
{
    return o->pageStruct.lineMapTable[0].fullLen == 0;
}

bool _isPrevLastLineWithEndLine(Obj * o)
{
    return o->pageStruct.prevLastLine.endsWithEndl;
}

size_t _updateLineMap(Obj * o, LineMap * lineMap, size_t lineBase)
{
    bool endOfTextFind = false;
    const unicode_t * const begin =
            LineBuffer_LoadText(o->modules, lineBase, o->modules->lineBuffer.size);

    LPM_TextLineMap textLineMap;
    if(TextOperator_analizeLine( o->modules->textOperator,
                                     begin,
                                     o->pageParams->charAmount,
                                     &textLineMap) )
        endOfTextFind = true;

    lineMap->fullLen    = (uint8_t)(textLineMap.nextLine    - begin);
    lineMap->payloadLen = (uint8_t)(textLineMap.printBorder - begin);
    lineMap->restLen    = o->pageParams->charAmount - textLineMap.lenInChr;
    lineMap->crc        = _calcLineCrc(o, lineMap);
    lineMap->endsWithEndl = textLineMap.endsWithEndl;
    return endOfTextFind;
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

    if(secondBegin > firstEnd)
        return false;

    if(secondEnd <= firstBegin)
        return false;

    return true;
}

bool _posWithinRange(size_t pos, size_t rangeBegin, size_t rangeEnd)
{
    return (pos >= rangeBegin) && (pos < rangeEnd);
}

LineMap * _findLineAtCursorPos(Obj * o, const SlcCurs * textCurs, size_t * pLineBase)
{
    LineMap * lineMap = o->pageStruct.lineMapTable;
    LineMap * const end = lineMap + o->pageParams->lineAmount;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    if(_isCurrPageLast(o))
    {
        size_t endOfText = TextStorage_endOfText(o->modules->textStorage);
        for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++)
        {
            if(_posWithinRange(textCurs->pos, lineBase, lineBase + lineMap->fullLen))
                break;

            if(lineBase + lineMap->fullLen == endOfText)
                break;
        }
    }
    else
    {
        for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++)
            if(_posWithinRange(textCurs->pos, lineBase, lineBase + lineMap->fullLen))
                break;
    }

    *pLineBase = lineBase;
    return lineMap != end ? lineMap : NULL;
}

void _updateDisplayCursorByTextCursor(Obj * o, const SlcCurs * curs)
{
    // Т.к. все остальные подфункции предполагают, что на странице должна быть
    //  хотя бы одна строка с ненулевой длиной, то полностью пустая страница
    //  является исключительным случаем - для нее своя ветка
    if(_isCurrPageEmpty(o))
    {
        _setDisplayCursorValueForEmptyPage(&o->displayCursor);
    }
    else
    {
        _setDisplayCursorInvalidValue(o);
        size_t lastNotEmptyLineIndex = _calcDisplayCursorByLineMapAnalizing(o, curs, &o->displayCursor);

        if(_isCurrPageLast(o))
        {
            _validateDisplayCursorPointIfInvalid(o, lastNotEmptyLineIndex, &o->displayCursor.begin);
            _validateDisplayCursorPointIfInvalid(o, lastNotEmptyLineIndex, &o->displayCursor.end);
        }
    }
}

size_t _calcDisplayCursorByLineMapAnalizing(Obj * o, const SlcCurs * txtCurs, DspCurs * dspCurs)
{
    size_t findPos = txtCurs->pos;
    LPM_Point * findPoint = &dspCurs->begin;

    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * const end = lineMap + o->pageParams->lineAmount;
    size_t lineIndex = 0;
    size_t lineBegin = _calcCurrPageFirstLineBase(o);
    for( ; lineMap != end; lineIndex++, lineMap++)
    {
        size_t lineEnd = lineBegin + lineMap->fullLen;

        if(lineMap->fullLen == 0)
            break;

        if(findPos < lineEnd)
        {
            findPoint->y = lineIndex;
            findPoint->x = findPos - lineBegin;
            if(findPoint == &dspCurs->begin)
            {
                findPoint = &dspCurs->end;
                findPos += txtCurs->len;
                if(findPos <= lineEnd)
                {
                    findPoint->y = lineIndex;
                    findPoint->x = findPos - lineBegin;
                    break;
                }
            }
            else
                break;
        }
        lineBegin = lineEnd;
    }

    // Т.к. при выходе из цикла lineIndex соответствует первой строке с нулевой
    //  длиной (т.е. текста дальше нет) или же lineIndex равен размеру таблицы
    //  строк (т.е. не было ни одной пустой строки), то индекс на единицу
    //  меньше соответствует последней непустой строке, что и должна вернуть
    //  функция
    return --lineIndex;
}

void _setDisplayCursorValueForEmptyPage(DspCurs * curs)
{
    curs->begin.x = 0;
    curs->begin.y = 0;
    curs->end.x = 0;
    curs->end.y = 0;
}

void _setDisplayCursorInvalidValue(Obj * o)
{
    o->displayCursor.begin.x = 0;
    o->displayCursor.begin.y = o->pageParams->lineAmount;
    o->displayCursor.end.x = 0;
    o->displayCursor.end.y = o->pageParams->lineAmount;
}

bool _isDisplayCursorPointInvalid(Obj * o, const LPM_Point * point)
{
    return point->y == o->pageParams->lineAmount;
}

void _validateDisplayCursorPointIfInvalid(Obj * o, size_t lineIndex, LPM_Point * point)
{
    if(_isDisplayCursorPointInvalid(o, point))
    {
        const LineMap * lineMap = o->pageStruct.lineMapTable + lineIndex;
        if(lineIndex == o->pageParams->lineAmount-1u)
        {
            point->y = lineIndex;
            point->x = lineMap->payloadLen;
        }
        else
        {
            if(lineMap->endsWithEndl)
            {
                point->y = lineIndex+1;
                point->x = 0;
            }
            else
            {
                point->y = lineIndex;
                point->x = lineMap->fullLen;
            }
        }
    }
}

//void _displayCursorToLineCursor(Obj * o, size_t lineIndex, size_t lineSize, SlcCurs * lineCursor)
//{
//    const size_t beginLine = o->displayCursor.begin.y;
//    const size_t endLine = o->displayCursor.end.y;
//    const size_t beginPos = o->displayCursor.begin.x;
//    const size_t endPos = o->displayCursor.end.x;
//    if(lineIndex > beginLine)
//    {
//        if(lineIndex > endLine)
//        {
//            lineCursor->pos = lineSize;
//            lineCursor->len = 0;
//        }
//        else if(lineIndex == endLine)
//        {
//            lineCursor->pos = 0;
//            lineCursor->len = endPos;
//        }
//        else
//        {
//            lineCursor->pos = 0;
//            lineCursor->len = lineSize;
//        }
//    }
//    else if(lineIndex == beginLine)
//    {
//        if(lineIndex == endLine)
//        {
//            lineCursor->pos = beginPos;
//            lineCursor->len = endPos - beginPos;
//        }
//        else
//        {
//            lineCursor->pos = beginPos;
//            lineCursor->len = lineSize;
//        }
//    }
//    else
//    {
//        lineCursor->pos = lineSize;
//        lineCursor->len = 0;
//    }
//}


PageStatus _moveFlagsToDisplayCursor(Obj * o, uint32_t moveFlags, DspCurs * dspCurs)
{
    PageStatus pageStatus = PAGE_CURR;
    uint32_t goal = _getGoalFlagValue(moveFlags);

    if(_getTypeFlagValue(moveFlags) == CURSOR_FLAG_MOVE)
    {
        if(goal == CURSOR_FLAG_PAGE)
            pageStatus = _moveCursorToPageBorder(o, _getBorderFlagValue(moveFlags), dspCurs);

        else if(goal == CURSOR_FLAG_LINE)
            _moveCursorToLineBorder(o, _getBorderFlagValue(moveFlags), dspCurs);

        else // if(goal == CURSOR_FLAG_CHAR)
            pageStatus = _moveCursorAtChar(o, _getDirectionFlagValue(moveFlags), dspCurs);
    }

    else // CURSOR_FLAG_SELECT
    {
        if(goal == CURSOR_FLAG_PAGE)
            _selectAllPage(o, dspCurs);

        else if(goal == CURSOR_FLAG_LINE)
            _selectAllLine(o, dspCurs);

        else // CURSOR_FLAG_CHAR
            _changeSelectionAtChar(o, _getDirectionFlagValue(moveFlags), dspCurs);
    }
    return pageStatus;
}

void _moveCursorToLineBorder(Obj * o, uint32_t borderFlag, DspCurs * dspCurs)
{
    if(borderFlag == CURSOR_FLAG_BEGIN)
    {
        _setDspPosToLineBegin(o, &dspCurs->begin);
        _setDspPosToLineBegin(o, &dspCurs->end);
    }
    else    // CURSOR_FLAG_END
    {
        _setDspPosToLineEnd(o, &dspCurs->begin);
        _setDspPosToLineEnd(o, &dspCurs->end);
    }

    return;

//    textCurs->len = 0;
//    const LineMap * lineMap = o->pageStruct.lineMapTable;
//    const LineMap * const end = lineMap + o->pageParams->lineAmount;
//    size_t lineBase = _calcCurrPageFirstLineBase(o);
//    for( ; lineMap != end; lineMap++)
//    {
//        if(_posWithinRange(textCurs->pos, lineBase, lineBase + lineMap->fullLen))
//            break;
//        lineBase += lineMap->fullLen;
//    }

//    size_t lineBase;
//    const LineMap * lineMap = _findLineAtCursorPos(o, textCurs, &lineBase);
//    test_print("!@#", textCurs->pos, lineMap - o->pageStruct.lineMapTable);

//    if(lineMap != NULL)
//    {
//        if(borderFlag == CURSOR_FLAG_BEGIN)
//        {
//            textCurs->pos = lineBase;
//            o->displayCursor.begin.x = 0;
//        }
//        else    // CURSOR_FLAG_END
//        {
//            if((lineMap - o->pageStruct.lineMapTable) == o->displayCursor.end.y)
//            {
//                o->displayCursor.begin.x = lineMap->payloadLen + lineMap->restLen - 1;
//                if(lineMap->restLen == 0)
//                {
//                    o->endLineSpaceAmount = 0;
//                    textCurs->pos = lineBase + lineMap->payloadLen-1;
//                }
//                else
//                {
//                    o->endLineSpaceAmount = lineMap->restLen-1;
//                    textCurs->pos = lineBase + lineMap->payloadLen;
//                }
//            }
//            else
//            {
//                o->displayCursor.begin.x = o->pageParams->charAmount-1;
//            }
//        }
//    }
//    else
//    {
//        o->displayCursor.begin.x = 0;
//        o->displayCursor.begin.y = 0;
//    }
}

PageStatus _moveCursorToPageBorder(Obj * o, uint32_t borderFlag, DspCurs * dspCurs)
{
    PageStatus pageStatus = PAGE_CURR;
    if(borderFlag == CURSOR_FLAG_BEGIN)
    {
        _setDspPosToPageBegin(o, &dspCurs->begin);
        _setDspPosToPageBegin(o, &dspCurs->end);
    }
    else if(borderFlag == CURSOR_FLAG_END)
    {
        _setDspPosToPageEnd(o, &dspCurs->begin);
        _setDspPosToPageEnd(o, &dspCurs->end);
    }
    else if(borderFlag == CURSOR_FLAG_NEXT)
    {
        if(!_isCurrPageLast(o))
        {
            _setDspPosToPageBegin(o, &dspCurs->begin);
            _setDspPosToPageBegin(o, &dspCurs->end);
            pageStatus = PAGE_NEXT;
        }
    }
    else //  CURSOR_FLAG_PREV
    {
        if(!_isCurrPageFirst(o))
        {
            _setDspPosToPageBegin(o, &dspCurs->begin);
            _setDspPosToPageBegin(o, &dspCurs->end);
            pageStatus = PAGE_PREV;
        }
    }
    return pageStatus;

//    textCurs->len = 0;
//    if(borderFlag == CURSOR_FLAG_BEGIN)
//    {
//        textCurs->pos = _calcCurrPageFirstLineBase(o);
//        o->displayCursor.begin.x = 0;
//        o->displayCursor.begin.y = 0;
//        return false;
//    }

//    if(borderFlag == CURSOR_FLAG_END)
//    {
////        textCurs->pos = _calcNextPageBase(o) +
////                o->pageStruct.lineMapTable[o->pageParams->lineAmount-1].payloadLen;

//        //const LineMap * lm = o->pageStruct.lineMapTable+o->pageParams->lineAmount-1;
//        textCurs->pos = _calcNextPageBase(o) +
//                o->pageStruct.lineMapTable[o->pageParams->lineAmount-1].payloadLen;
//        o->displayCursor.begin.y = _calcPageLastLineIndex(o);
//        o->displayCursor.end.y   = o->displayCursor.begin.y;
//        _moveCursorToLineBorder(o, CURSOR_FLAG_END, textCurs);

////        o->displayCursor.begin.x = lm->payloadLen + lm->restLen - 1;
////        o->displayCursor.begin.y = o->pageParams->lineAmount-1;

////        o->endLineSpaceAmount = lm->restLen == 0 ? 0 : lm->restLen-1;

//        return false;
//    }

//    if(borderFlag == CURSOR_FLAG_NEXT)
//    {
//        if(_isCurrPageLast(o))
//            return false;
//        textCurs->pos = _calcNextPageBase(o)+
//                o->pageStruct.lineMapTable[o->pageParams->lineAmount-1].fullLen;
//        _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
//    }
//    else // CURSOR_FLAG_PREV
//    {
//        if(_isCurrPageFirst(o))
//            return false;
//        textCurs->pos = o->pageStruct.base;
//        _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
//        textCurs->pos = _calcCurrPageFirstLineBase(o);
//    }

//    _updateDisplayCursorByTextCursor(o, textCurs);
//    _saveTextCursor(o, textCurs);
//    return true;
}

PageStatus _moveCursorAtChar(Obj * o, uint32_t dirFlags, DspCurs * dspCurs)
{
    PageStatus pageStatus = PAGE_CURR;
    bool textSelected = _textSelected(dspCurs);

    if(dirFlags == CURSOR_FLAG_LEFT)
    {
        if(!textSelected)
        {
            if(_decDspXPos(o, &dspCurs->begin))
            {
                if(!_isCurrPageFirst(o))
                {
                    pageStatus = PAGE_PREV;
                    _setDspPosToPageEnd(o, &dspCurs->begin);
                }
            }
        }
        _copyDspPoint(&dspCurs->end, &dspCurs->begin);
    }
    else if(dirFlags == CURSOR_FLAG_RIGHT)
    {
        if(!textSelected)
        {
            if(_incDspXPos(o, &dspCurs->end))
            {
                if(!_isCurrPageLast(o))
                {
                    pageStatus = PAGE_NEXT;
                    _setDspPosToPageBegin(o, &dspCurs->end);
                }
            }
        }
        _copyDspPoint(&dspCurs->begin, &dspCurs->end);
    }
    else if(dirFlags == CURSOR_FLAG_UP)
    {
        if(!textSelected)
        {
            if(_decDspYPos(o, &dspCurs->begin))
            {
                if(!_isCurrPageFirst(o))
                {
                    pageStatus = PAGE_PREV;
                    dspCurs->begin.y = o->pageParams->lineAmount-1;
                }
            }
        }
        _copyDspPoint(&dspCurs->end, &dspCurs->begin);
    }
    else //if(dirFlags == CURSOR_FLAG_DOWN)
    {
        if(!textSelected)
        {
            if(_incDspYPos(o, &dspCurs->end))
            {
                if(!_isCurrPageLast(o))
                {
                    pageStatus = PAGE_NEXT;
                    dspCurs->end.y = 0;
                }
            }
        }
        _copyDspPoint(&dspCurs->begin, &dspCurs->end);
    }

    return pageStatus;

//    if(dirFlags == CURSOR_FLAG_LEFT)
//    {
//        if(textCurs->len > 0)
//            textCurs->len = 0;
//        else
//            textCurs->pos = _movePosLeftAtChar(o, textCurs->pos);
//    }
//    else if(dirFlags == CURSOR_FLAG_RIGHT)
//    {
//        if(textCurs->len > 0)
//        {
//            textCurs->pos += textCurs->len;
//            textCurs->len = 0;
//        }
//        else
//            textCurs->pos = _movePosRightAtChar(o, textCurs->pos);
//    }
//    else if(dirFlags == CURSOR_FLAG_UP)
//    {
//        if(textCurs->len > 0)
//            textCurs->len = 0;
//        else
//        {
//            textCurs->pos = _movePosUpAtChar(o, textCurs->pos, &o->displayCursor.begin);
//        }
//    }
//    else //if(dirFlags == CURSOR_FLAG_DOWN)
//    {
//        if(textCurs->len > 0)
//        {
//            textCurs->pos += textCurs->len;
//            textCurs->len = 0;
//        }
//        else
//        {
//            textCurs->pos = _movePosDownAtChar(o, textCurs->pos, &o->displayCursor.begin);
//        }
//    }
}

void _selectAllPage(Obj * o, DspCurs * dspCurs)
{
    o->selectBackward = false;
    _setDspPosToPageBegin(o, &dspCurs->begin);
    _setDspPosToPageEnd(o, &dspCurs->end);
    _incDspXPos(o, &dspCurs->end);
//    return;

//    textCurs->pos = _calcCurrPageFirstLineBase(o);
//    textCurs->len = _calcCurrPageLen(o);
}

void _selectAllLine(Obj * o, DspCurs * dspCurs)
{
    o->selectBackward = false;
    _setDspPosToLineBegin(o, &dspCurs->begin);
    _setDspPosToLineEnd(o, &dspCurs->end);
    _incDspXPos(o, &dspCurs->end);
    return;
//    const LineMap * lineMap = o->pageStruct.lineMapTable;
//    const LineMap * const end = lineMap + o->pageParams->lineAmount;
//    size_t lineBase = _calcCurrPageFirstLineBase(o);
//    for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++)
//        if(_posWithinRange(textCurs->pos, lineBase, lineBase + lineMap->fullLen))
//            break;
//    size_t lineBase;
//    const LineMap * lineMap = _findLineAtCursorPos(o, textCurs, &lineBase);

//    if(lineMap != NULL)
//    {
//        textCurs->pos = lineBase;
//        textCurs->len = lineMap->fullLen;
//    }
}

void _changeSelectionAtChar(Obj * o, uint32_t dirFlags, DspCurs * dspCurs)
{
    _normalizeSelectionDirAndSwapDspPointsIfNeed(o, dirFlags, dspCurs);

    if(dirFlags == CURSOR_FLAG_LEFT)
    {
        if(o->selectBackward)
            _decDspXPos(o, &dspCurs->begin);
        else
            _decDspXPos(o, &dspCurs->end);
    }
    else if(dirFlags == CURSOR_FLAG_RIGHT)
    {
        if(o->selectBackward)
            _incDspXPos(o, &dspCurs->begin);
        else
            _incDspXPos(o, &dspCurs->end);
    }
    else if(dirFlags == CURSOR_FLAG_UP)
    {
        if(o->selectBackward)
            _decDspYPos(o, &dspCurs->begin);
        else
            _decDspYPos(o, &dspCurs->end);
    }
    else //if(dirFlags == CURSOR_FLAG_DOWN)
    {
        if(o->selectBackward)
            _incDspYPos(o, &dspCurs->begin);
        else
            _incDspYPos(o, &dspCurs->end);
    }

//    return;

//    size_t begin;
//    size_t end;
//    const LPM_Point * dspPoint;

//    if(o->selectBackward)
//    {
//        end = textCurs->pos;
//        begin = end + textCurs->len;
//        dspPoint = &o->displayCursor.begin;
//    }
//    else
//    {
//        begin = textCurs->pos;
//        end = begin + textCurs->len;
//        dspPoint = &o->displayCursor.end;
//    }

//    if(dirFlags == CURSOR_FLAG_LEFT)
//    {
//        end = _movePosLeftAtChar(o, end);
//    }
//    else if(dirFlags == CURSOR_FLAG_RIGHT)
//    {
//        end = _movePosRightAtChar(o, end);
//    }
//    else if(dirFlags == CURSOR_FLAG_UP)
//    {
//        end = _movePosUpAtChar(o, end, dspPoint);
//    }
//    else //if(dirFlags == CURSOR_FLAG_DOWN)
//    {
//        end = _movePosDownAtChar(o, end, dspPoint);
//    }

//    size_t pageBegin = _calcCurrPageFirstLineBase(o);
//    size_t pageEnd = pageBegin + _calcCurrPageLen(o);
//    if(end < pageBegin)
//        end = pageBegin;
//    if(end >= pageEnd)
//        end = pageEnd;

//    if(o->selectBackward)
//    {
//        if(begin <= end)
//        {
//            o->selectBackward = false;
//            textCurs->pos = begin;
//            textCurs->len = end - begin;
//        }
//        else
//        {
//            textCurs->pos = end;
//            textCurs->len = begin - end;
//        }
//    }
//    else
//    {
//        if(end < begin)
//        {
//            o->selectBackward = true;
//            textCurs->pos = end;
//            textCurs->len = begin - end;
//        }
//        else
//        {
//            textCurs->pos = begin;
//            textCurs->len = end - begin;
//        }
//    }
}

void _normalizeSelectionDirAndSwapDspPointsIfNeed(Obj * o, uint32_t dirFlags, DspCurs * dspCurs)
{
    if(!_textSelected(dspCurs))
    {
        if(dirFlags == CURSOR_FLAG_LEFT || dirFlags == CURSOR_FLAG_UP)
            o->selectBackward = true;
        else
            o->selectBackward = false;
    }
    else
    {
        int yDiff = dspCurs->end.y - dspCurs->begin.y;
        if(yDiff == 0)
        {
            if(o->selectBackward && (dirFlags == CURSOR_FLAG_DOWN))
            {
                o->selectBackward = !o->selectBackward;
                _swapDspPoints(dspCurs);
            }
            else if((!o->selectBackward) && (dirFlags == CURSOR_FLAG_UP))
            {
                o->selectBackward = !o->selectBackward;
                _swapDspPoints(dspCurs);
            }
        }
        else if(yDiff == 1)
        {
            if(o->selectBackward)
            {
                if(dspCurs->begin.x > dspCurs->end.x)
                {
                    if(dirFlags == CURSOR_FLAG_DOWN)
                    {
                        o->selectBackward = !o->selectBackward;
                        _swapDspPoints(dspCurs);
                    }
                }
            }
            else
            {
                if(dspCurs->begin.x > dspCurs->end.x)
                {
                    if(dirFlags == CURSOR_FLAG_UP)
                    {
                        o->selectBackward = !o->selectBackward;
                        _swapDspPoints(dspCurs);
                    }
                }
            }
        }
    }
}

bool _textSelected(const DspCurs * dspCurs)
{
    return (dspCurs->begin.x != dspCurs->end.x) ||
           (dspCurs->begin.y != dspCurs->end.y);
}

bool _incDspXPos(Obj * o, LPM_Point * dspPoint)
{
    bool endReatched = false;
    if(dspPoint->x < o->pageParams->charAmount)
    {
        dspPoint->x++;
    }
    else if(dspPoint->y < o->pageParams->lineAmount-1)
    {
        dspPoint->x = 0;
        dspPoint->y++;
    }
    else
    {
        endReatched = true;
    }
    return endReatched;
}

bool _decDspXPos(Obj * o, LPM_Point * dspPoint)
{
    bool beginReatched = false;
    if(dspPoint->x > 0)
    {
        dspPoint->x--;
    }
    else if(dspPoint->y > 0)
    {
        dspPoint->x = o->pageParams->charAmount;
        dspPoint->y--;
    }
    else
    {
        beginReatched = true;
    }
    return beginReatched;
}

bool _incDspYPos(Obj * o, LPM_Point * dspPoint)
{
    bool endReatched = false;
    if(dspPoint->y < o->pageParams->lineAmount-1)
        dspPoint->y++;
    else
        endReatched = true;
    return endReatched;
}


bool _decDspYPos(Obj * o, LPM_Point * dspPoint)
{
    (void)o;
    bool beginReatched = false;
    if(dspPoint->y > 0)
        dspPoint->y--;
    else
        beginReatched = true;
    return beginReatched;
}

void _setDspPosToLineBegin(Obj * o, LPM_Point * dspPoint)
{
    (void)(o);
    dspPoint->x = 0;
}

void _setDspPosToLineEnd(Obj * o, LPM_Point * dspPoint)
{
    dspPoint->x = o->pageParams->charAmount-1;
}

void _setDspPosToPageBegin(Obj * o, LPM_Point * dspPoint)
{
    (void)(o);
    dspPoint->x = 0;
    dspPoint->y = 0;
}

void _setDspPosToPageEnd(Obj * o, LPM_Point * dspPoint)
{
    dspPoint->x = o->pageParams->charAmount-1;
    dspPoint->y = o->pageParams->lineAmount-1;
}

void _copyDspPoint(LPM_Point * dst, const LPM_Point * src)
{
    dst->x = src->x;
    dst->y = src->y;
}

void _swapDspPoints(DspCurs * dspCurs)
{
    size_t tmp = dspCurs->begin.x;
    dspCurs->begin.x = dspCurs->end.x;
    dspCurs->end.x = tmp;
    tmp = dspCurs->begin.y;
    dspCurs->begin.y = dspCurs->end.y;
    dspCurs->end.y = tmp;
}

void _changePageByStatus(Obj * o, PageStatus ps)
{
    if(ps == PAGE_NEXT)
        _changePageIfNotOnCurrTextPosition(o, _calcNextPageBase(o) +
                                               o->pageStruct.lineMapTable[o->pageParams->lineAmount-1].fullLen);

    if(ps == PAGE_PREV)
        _changePageIfNotOnCurrTextPosition(o, o->pageStruct.base);
}

void _calcTextCursorByDisplayCursorWhenPageChangedAndChangPage
        (Obj * o, uint32_t moveFlags, SlcCurs * textCurs)
{
    uint32_t goal = _getGoalFlagValue(moveFlags);
    if(goal == CURSOR_FLAG_PAGE)
    {
        uint32_t border =  _getBorderFlagValue(moveFlags);
        if(border == CURSOR_FLAG_NEXT)
        {
            if(!_isCurrPageLast(o))
            {
                textCurs->len = 0;
                textCurs->pos = _calcNextPageBase(o) +
                        o->pageStruct.lineMapTable[o->pageParams->lineAmount-1].fullLen;
                _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
            }
        }
        else // CURSOR_FLAG_PREV
        {
            if(!_isCurrPageFirst(o))
            {
                textCurs->len = 0;
                textCurs->pos = o->pageStruct.base;
                _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
                textCurs->pos = _calcCurrPageFirstLineBase(o);
            }
        }
    }
    else if(goal == CURSOR_FLAG_CHAR)
    {
        uint32_t dir =  _getDirectionFlagValue(moveFlags);
        if(dir == CURSOR_FLAG_UP)
        {
            if(!_isCurrPageFirst(o))
            {
                o->displayCursor.begin.y = o->pageParams->lineAmount-1;
                o->displayCursor.end.y = o->pageParams->lineAmount-1;
                textCurs->len = 0;
                textCurs->pos = o->pageStruct.base;
                _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
                unicode_t * pchr = LineBuffer_LoadText(o->modules, textCurs->pos, o->modules->lineBuffer.size);
                textCurs->pos += TextOperator_nextNChar(o->modules->textOperator, pchr, o->displayCursor.begin.x)-pchr;
            }
            // Пред.стр: посл. строка
        }
        else if(dir == CURSOR_FLAG_LEFT)
        {
            if(!_isCurrPageFirst(o))
            {
            }
            // Пред.стр: конец
        }
        else if(CURSOR_FLAG_DOWN)
        {
            if(!_isCurrPageLast(o))
            {
            }
            // След.стр: перв. строка
        }
        else // if(CURSOR_FLAG_RIGHT)
        {
            if(!_isCurrPageLast(o))
            {
            }
            // Пред.стр: начало
        }
    }
}

void _displayCursorToTextCursor(Obj * o, SlcCurs * textCursor)
{
    size_t cursBegin = textCursor->pos;
    size_t cursEnd = cursBegin + textCursor->len;

    const LineMap * lm = o->pageStruct.lineMapTable;
    const LineMap * const end = lm + o->pageParams->lineAmount;
    size_t lineIndex = 0;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    for( ; lm < end; lineBase += lm->fullLen, lm++, lineIndex++)
    {
        if(lineIndex == o->displayCursor.begin.y)
        {
            cursBegin = _calcTextPosByDisplayXPos(o, lineBase, lm, o->displayCursor.begin.x);
            _calcSpaceAmount(o, lm, o->displayCursor.begin.x);
        }

        if(lineIndex == o->displayCursor.end.y)
        {
            cursEnd = _calcTextPosByDisplayXPos(o, lineBase, lm, o->displayCursor.end.x);
        }
    }
    textCursor->pos = cursBegin;
    textCursor->len = cursEnd - cursBegin;
}

bool _dspXPosIsAfterLinePayloadPart(Obj * o, const LineMap * lm, size_t xPos)
{
    int diff = o->pageParams->charAmount - lm->restLen;

    if(diff > 0)
        return xPos > (size_t)diff;
    return true;
}

size_t _calcTextPosByDisplayXPos(Obj * o, size_t lineBase, const LineMap * lm, size_t xPos)
{
    size_t lineX;
    if(_dspXPosIsAfterLinePayloadPart(o, lm, xPos))
    {
        lineX = lm->payloadLen;
    }
    else
    {
        unicode_t * pchr = LineBuffer_LoadText
                (o->modules, lineBase, o->modules->lineBuffer.size);

        lineX = TextOperator_nextNChar(o->modules->textOperator, pchr, xPos)
                - pchr;
    }
    return lineBase + lineX;
}

size_t _calcSpaceAmount(Obj * o, const LineMap * lm, size_t xPos)
{
    return _dspXPosIsAfterLinePayloadPart(o, lm, xPos) ?
                lm->restLen + xPos - o->pageParams->charAmount : 0;
}

void _textCursorToDisplayCursor(Obj * o, DspCurs * dspCurs, const SlcCurs * txtCurs)
{
    _setInvalidDisplayCursorValue(o, dspCurs);
    size_t firstEmptyLineIndex =
            _calcDisplayCursorYValues(o, dspCurs, txtCurs);

    _validateDisplayPointIfInvalid(o, &dspCurs->begin, firstEmptyLineIndex);
    _validateDisplayPointIfInvalid(o, &dspCurs->end,   firstEmptyLineIndex);

    _calcDisplayPointXValue(o, &dspCurs->begin, txtCurs->pos);
    _calcDisplayPointXValue(o, &dspCurs->end, txtCurs->pos + txtCurs->len);

    // Установить некорректное значение курсора дисплея
    // Вычислить позицию курсора и номер первой пустой строки
    // Проверить курсор на корректность
    // Если некорректен, то:
    //  Если в строке до курсора был символ перевода строки, то установить курсор в конец строки до первой пустой
    //  Иначе - установить курсор в начало первой пустой строки
}

void _setInvalidDisplayCursorValue(Obj * o, DspCurs * dspCurs)
{
    dspCurs->begin.x = 0;
    dspCurs->end.x = 0;
    dspCurs->begin.y = o->pageParams->lineAmount;
    dspCurs->end.y = o->pageParams->lineAmount;
}

size_t _calcDisplayCursorYValues(Obj * o, DspCurs * dspCurs, const SlcCurs * txtCurs)
{
    size_t findPos = txtCurs->pos;
    LPM_Point * findPoint = &dspCurs->begin;

    const LineMap * lm = o->pageStruct.lineMapTable;
    const LineMap * const end = lm + o->pageParams->lineAmount;
    size_t lineEnd = _calcCurrPageFirstLineBase(o);
    size_t lineIndex = 0;
    for( ; lm != end; lm++, lineIndex++)
    {
        if(lm->fullLen == 0)
            break;

        lineEnd += lm->fullLen;

        if(findPos < lineEnd)
        {
            findPoint->y = lineIndex;

            if(findPoint == &dspCurs->begin)
            {
                findPoint = &dspCurs->end;
                findPos  += txtCurs->len;

                if(findPos < lineEnd)
                {
                    findPoint->y = lineIndex;
                    break;
                }
            }
            else
                break;
        }
    }

    return lineIndex;
}

void _validateDisplayPointIfInvalid(Obj * o, LPM_Point * point, size_t firstEmptyLineIndex)
{
    if(point->y == o->pageParams->lineAmount)
    {
        const LineMap * const tbl = o->pageStruct.lineMapTable;

        if(firstEmptyLineIndex == 0)
            point->y = 0;

        else if(tbl[firstEmptyLineIndex-1].endsWithEndl)
            point->y = firstEmptyLineIndex;

        else
            point->y = firstEmptyLineIndex-1;
    }
}

void _calcDisplayPointXValue(Obj * o, LPM_Point * point, size_t txtPos)
{
    const LineMap * lm = o->pageStruct.lineMapTable + point->y;
    if(lm->payloadLen == 0)
    {
        point->x = 0;
        return;
    }

    size_t lineBase = _calcLineBase(o, point->y);
    if(txtPos < lineBase)
    {
        point->x = 0;
        return;
    }

    unicode_t * pchr = LineBuffer_LoadText(o->modules, lineBase, lm->fullLen);
    point->x = TextOperator_calcChrAmount
            (o->modules->textOperator, pchr, pchr + txtPos-lineBase);
}



size_t _movePosLeftAtChar(Obj * o, size_t pos)
{
    if(pos > 0)
    {
        unicode_t * pchr = LineBuffer_LoadTextBack(o->modules, pos, o->modules->charBuffer.size);
        pos -= pchr - TextOperator_prevChar(o->modules->textOperator, pchr);
    }
    return pos;
}

size_t _movePosRightAtChar(Obj * o, size_t pos)
{
    unicode_t * pchr = LineBuffer_LoadText(o->modules, pos, o->modules->charBuffer.size);
    pos += TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;
    return pos;
}

size_t _movePosUpAtChar(Obj * o, size_t textPos, const LPM_Point * dspPoint)
{
    size_t lineIndex = dspPoint->y;
    size_t currLineX = dspPoint->x;
    if(lineIndex > 0 || !_isCurrPageFirst(o))
    {
        const LineMap * prevLine = lineIndex == 0 ?
                    &o->pageStruct.prevLastLine :
                    o->pageStruct.lineMapTable+lineIndex-1;
        if(currLineX > prevLine->payloadLen)
        {
            textPos -= currLineX;
            textPos -= prevLine->fullLen;
            textPos += prevLine->payloadLen;
        }
        else
        {
            textPos -= currLineX;
            const unicode_t * pchr = LineBuffer_LoadText(o->modules, textPos, o->modules->lineBuffer.size );
            size_t chrAmount = TextOperator_calcChrAmount(o->modules->textOperator, pchr, pchr+currLineX);
            textPos -= prevLine->fullLen;
            pchr = LineBuffer_LoadText(o->modules, textPos, o->modules->lineBuffer.size);
            textPos += TextOperator_nextNChar(o->modules->textOperator, pchr, chrAmount) - pchr;
        }
    }
    return textPos;
}

size_t _movePosDownAtChar(Obj * o, size_t textPos, const LPM_Point * dspPoint)
{
    size_t lineIndex = dspPoint->y;
    size_t currLineX = dspPoint->x;

    LineMap nextLine;
    const LineMap * currLine = o->pageStruct.lineMapTable+lineIndex;

    if(lineIndex == o->pageParams->lineAmount-1u)
    {
        size_t nextLineBase = _calcNextPageBase(o) + currLine->fullLen;
        _updateLineMap(o, &nextLine, nextLineBase);
    }
    else
    {
        _copyLineMap(&nextLine, o->pageStruct.lineMapTable+lineIndex+1);
    }

    //const LineMap * nextLine = currLine+1;
    if(currLineX > nextLine.payloadLen)
    {
        textPos -= currLineX;
        textPos += currLine->fullLen;
        textPos += nextLine.payloadLen;
    }
    else
    {
        textPos -= currLineX;
        const unicode_t * pchr = LineBuffer_LoadText(o->modules, textPos, o->modules->lineBuffer.size );
        size_t chrAmount = TextOperator_calcChrAmount(o->modules->textOperator, pchr, pchr + currLineX);
        textPos += currLine->fullLen;
        pchr = LineBuffer_LoadText(o->modules, textPos, o->modules->lineBuffer.size);
        textPos += TextOperator_nextNChar(o->modules->textOperator, pchr, chrAmount) - pchr;
    }
    return textPos;
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

void _saveTextCursor(Obj * o, const SlcCurs * textCurs)
{
    memcpy(&o->textCursor, textCurs, sizeof(SlcCurs));
}

void _updateLineChangedFlagsWhenTextCursorChanged(Obj * o, const SlcCurs * newTextCursor)
{
    // Симметрическая разность - области двух множеств, которые принадлежат
    //  только какому-то одному из них. Т.е. это объединение минус пересечение.
    SlcCurs symDiff1;
    SlcCurs symDiff2;
    _calcSymmetricDifference(&o->textCursor, newTextCursor, &symDiff1, &symDiff2);
    _updateLineChangedFlagsByTextCursorChanging(o, &symDiff1, &symDiff2);
}

void _updateLineChangedFlagsByDisplayCursor(Obj * o, const DspCurs * dspCurs)
{
    SlcCurs linDspBefore;
    SlcCurs linDspAfter;
    SlcCurs symDif1;
    SlcCurs symDif2;

    _displayCursorToLinearCursor(o, &linDspBefore, &o->displayCursor);
    _displayCursorToLinearCursor(o, &linDspAfter, dspCurs);
    _calcSymmetricDifference(&linDspBefore, &linDspAfter, &symDif1, &symDif2);
    _setLineChangedFlagsBySymmetricDifferencePart(o, &symDif1);
    _setLineChangedFlagsBySymmetricDifferencePart(o, &symDif2);
}

void _displayCursorToLinearCursor(Obj * o, SlcCurs * slcCurs, const DspCurs * dspCurs)
{
    size_t begin = dspCurs->begin.y * o->pageParams->charAmount + dspCurs->begin.x;
    size_t end   = dspCurs->end.y   * o->pageParams->charAmount + dspCurs->end.x;
    slcCurs->pos = begin;
    slcCurs->len = end - begin;
}

void _setLineChangedFlagsBySymmetricDifferencePart(Obj * o, const SlcCurs * symDifPart)
{
    size_t begin = symDifPart->pos;
    size_t end   = begin + symDifPart->len;

    begin = symDifPart->pos / o->pageParams->charAmount;
    end   = (symDifPart->pos + symDifPart->len) / o->pageParams->charAmount;

    uint32_t flags = 0;
    uint32_t currFlag = 1 << begin;
    for(size_t i = begin; i <= end; i++, currFlag <<= 1)
        flags |= currFlag;

    o->lineChangedFlags |= flags;
}

void _copyDisplayCursor(DspCurs * dst, const DspCurs * src)
{
    dst->begin.x = src->begin.x;
    dst->begin.y = src->begin.y;
    dst->end.x = src->end.x;
    dst->end.y = src->end.y;
}

void _calcSymmetricDifference (const SlcCurs * src1, const SlcCurs * src2, SlcCurs * dst1, SlcCurs * dst2)
{
    // Чтобы расчитать симметрическую разность, нужно отсортировать абсолютные
    //  значения позиций начала и конца каждого интервала, первые два из них
    //  будут в первом результирующем интервале, остальные - во втором:
    //   Интервал 1:  ||||||||||||
    //   Интервал 2:        ||||||||||||
    //   Разность:    ||||||      ||||||

    // Сортируем так:
    //  1. Сначала определяем, какой интервал левее
    //  2. Далее, если конец левого <= начала правого, то позиции уже
    //     отсортированы
    //  3. Иначе начало второго лежит до конца первого. Теперь проверяем, что
    //     конец правого < конца левого и ставим интервалы.

    // Еще до начала сортировки: если длина интервала равна 0, то нужно ее
    //  принудительно установить в 1. Это потому что 0 соответствует нулевой
    //  длине области выделения (т.е. простому курсорву ввода). Однако на
    //  экране курсор все-таки одно знакоместо занимает, а значит и область
    //  нужно увеличить на 1

    size_t leftBegin;
    size_t leftEnd;
    size_t rightBegin;
    size_t rightEnd;

    // 1й: |||||||
    // 2й:     |||||||
    if(src1->pos <= src2->pos)
    {
        _normalizeTextCursorAndFillBeginEnd(src1, &leftBegin, &leftEnd);
        _normalizeTextCursorAndFillBeginEnd(src2, &rightBegin, &rightEnd);
    }
    // 1й:     |||||||
    // 2й: |||||||
    else
    {
        _normalizeTextCursorAndFillBeginEnd(src2, &leftBegin, &leftEnd);
        _normalizeTextCursorAndFillBeginEnd(src1, &rightBegin, &rightEnd);
    }

    // левый:  ||||||
    // правый:         ||||||
    // итог:   ||||||  ||||||
    if(leftEnd <= rightBegin)
    {
        dst1->pos = leftBegin;
        dst1->len = leftEnd - leftBegin;
        dst2->pos = rightBegin;
        dst2->len = rightEnd - rightBegin;
    }
    // левый:  ||||||||||||||
    // правый:     ||||||
    // итог:   ||||      ||||
    else if(rightEnd < leftEnd)
    {
        dst1->pos = leftBegin;
        dst1->len = rightBegin - leftBegin;
        dst2->pos = rightEnd;
        dst2->len = leftEnd - rightEnd;
    }
    // левый:  ||||||||
    // правый:     ||||||||||
    // итог:   ||||    ||||||
    else
    {
        dst1->pos = leftBegin;
        dst1->len = rightBegin - leftBegin;
        dst2->pos = leftEnd;
        dst2->len = rightEnd - leftEnd;
    }
}

void _normalizeTextCursorAndFillBeginEnd(const SlcCurs * curs, size_t * begin, size_t * end)
{
    *begin = curs->pos;
    *end   = curs->pos + (curs->len > 0 ? curs->len : 1);
}

void _updateLineChangedFlagsByTextCursorChanging(Obj * o, const SlcCurs * symDiff1, const SlcCurs * symmDiff2)
{
    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * const end = lineMap + o->pageParams->lineAmount;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    size_t lineIndex = 0;
    for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++, lineIndex++)
    {
        if(lineMap->fullLen == 0)
        {
            _setLineChangedFlag(o, lineIndex);
            _setLineChangedFlag(o, lineIndex+1);
            break;
        }
        if(_changedTextCrossesLine(symDiff1, lineBase, lineMap->fullLen))
            _setLineChangedFlag(o, lineIndex);
        if(_changedTextCrossesLine(symmDiff2, lineBase, lineMap->fullLen))
            _setLineChangedFlag(o, lineIndex);
    }
}

void _displayLine(Obj * o, size_t lineIndex, size_t lineOffset)
{
    Unicode_Buf lineBuf;
    const LineMap * lineMap = o->pageStruct.lineMapTable + lineIndex;
    SlcCurs lineCursor;

    // 1. Определить размер строки, т.е. размер нагрузки + размер остатка.
    //    Так же этого достаточно, чтобы заполнить буфер строки
    // 2. Так же нужно определить x-точки курсора дисплея, если соответствующие
    //    y-точки совпадают с индексом строки

    _readDisplayedLineToBuffer(o, lineMap, lineOffset, &lineBuf);

    _displayCursorToLineCursor(o, lineIndex, &lineCursor);

    LPM_UnicodeDisplay_writeLine( o->display,
                                  lineIndex,
                                  &lineBuf,
                                  &lineCursor);
}

void _readDisplayedLineToBuffer(Obj * o, const LineMap * lineMap, size_t lineOffset, Unicode_Buf * lineBuf)
{
    lineBuf->data = LineBuffer_LoadText(o->modules, lineOffset, lineMap->payloadLen);
    lineBuf->size = lineMap->payloadLen + lineMap->restLen;

    unicode_t * pchr      = lineBuf->data + lineMap->payloadLen;
    unicode_t * const end = pchr + lineMap->restLen;
    for( ; pchr != end; pchr++)
        *pchr = chrSpace;
}

void _displayCursorToLineCursor(Obj * o, size_t lineIndex, SlcCurs * lineCursor)
{
    const LineMap * lineMap = o->pageStruct.lineMapTable + lineIndex;
    const size_t lineSize  = lineMap->payloadLen + lineMap->restLen;
    const size_t lineBegin = o->displayCursor.begin.y;
    const size_t lineEnd   = o->displayCursor.end.y;

    if(lineIndex > lineBegin)
    {
        if(lineIndex > lineEnd)
        {
            lineCursor->pos = lineSize;
            lineCursor->len = 0;
        }
        else if(lineIndex == lineEnd)
        {
            lineCursor->pos = 0;
            lineCursor->len = _calcPositionOfDisplayXvalue(o, o->displayCursor.end.x);
        }
        else
        {
            lineCursor->pos = 0;
            lineCursor->len = lineSize;
        }
    }
    else if(lineIndex == lineBegin)
    {
        size_t beginPos = _calcPositionOfDisplayXvalue(o, o->displayCursor.begin.x);
        if(lineIndex == lineEnd)
        {
            size_t endPos = _calcPositionOfDisplayXvalue(o, o->displayCursor.end.x);
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

size_t _calcPositionOfDisplayXvalue(Obj * o, size_t x)
{
    const unicode_t * pchr = TextOperator_nextNChar
                ( o->modules->textOperator,
                  o->modules->lineBuffer.data,
                  x );
    return pchr - o->modules->lineBuffer.data;
}


void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset)
{
    Unicode_Buf buf =
    {
        .data = o->modules->lineBuffer.data,
        .size = lineMap->payloadLen,
    };
    TextStorage_read(o->modules->textStorage, lineOffset, &buf);

    unicode_t * pchr = buf.data + lineMap->payloadLen;
    unicode_t * const end = pchr + lineMap->restLen;
    for( ; pchr != end; pchr++)
        *pchr = chrSpace;
}

size_t _calcPageLastLineIndex(Obj * o)
{
    if(_isCurrPageLast(o))
    {
        LineMap * clm = o->pageStruct.lineMapTable;

        if(clm->fullLen == 0)
            return 0;

        LineMap * plm = clm;
        clm++;
        size_t lineIndex = 1;
        LineMap * const end = clm + o->pageParams->lineAmount;
        for( ; clm != end; lineIndex++, clm++, plm++)
        {
            if(clm->fullLen == 0)
            {
                if(plm->endsWithEndl)
                    return lineIndex;
                return lineIndex-1;
            }
        }
    }

    return o->pageParams->lineAmount-1;
}
