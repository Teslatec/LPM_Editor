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

static void _displayCursorToTextCursor(Obj * o, SlcCurs * textCursor);

static bool _dspXPosIsAfterLinePayloadPart(Obj * o, const LineMap * lm, size_t xPos);
static size_t _calcTextPosByDisplayXPos(Obj * o, size_t lineBase, const LineMap * lm, size_t xPos);
static void _resetAddChars(Obj * o);
static void _calcAddSpaceAmount(Obj * o, const LineMap * lm, size_t xPos);
static void _calcAddLineAmount(Obj * o, size_t firstEmptyLineIndex);

static void _textCursorToDisplayCursor(Obj * o, DspCurs * dspCurs, const SlcCurs * txtCurs);
static void _setInvalidDisplayCursorValue(Obj * o, DspCurs * dspCurs);
static size_t _calcDisplayCursorYValues(Obj * o, DspCurs * dspCurs, const SlcCurs * txtCurs);
static void _validateDisplayPointIfInvalid(Obj * o, LPM_Point * point, size_t firstEmptyLineIndex);
static void _calcDisplayPointXValue(Obj * o, LPM_Point * point, size_t txtPos);

static uint32_t _getTypeFlagValue(uint32_t flags);
static uint32_t _getGoalFlagValue(uint32_t flags);
static uint32_t _getDirectionFlagValue(uint32_t flags);
static uint32_t _getBorderFlagValue(uint32_t flags);

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

static void _displayLine(Obj * o, size_t lineIndex, size_t lineOffset);

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
          const LPM_SelectionCursor * txtCurs )
{
    DspCurs dspCurs;
    _resetAddChars(o);
    _setFirstPageInGroup(o, 0);
    _changePageIfNotOnCurrTextPosition(o, txtCurs->pos);
    _textCursorToDisplayCursor(o, &dspCurs, txtCurs);
    _copyDisplayCursor(&o->displayCursor, &dspCurs);
}

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * txtCurs )
{
    DspCurs dspCurs;

    _resetAddChars(o);
    _resetAllLineChangedFlags(o);

    _updateLinesMap(o);
    _copyDisplayCursor(&dspCurs, &o->displayCursor);

    if(!_changePageIfNotOnCurrTextPosition(o, txtCurs->pos))
    {
        _textCursorToDisplayCursor(o, &dspCurs, txtCurs);
        _updateLineChangedFlagsByDisplayCursor(o, &dspCurs);
    }
    else
    {
        _textCursorToDisplayCursor(o, &dspCurs, txtCurs);
    }

    _copyDisplayCursor(&o->displayCursor, &dspCurs);
}


void PageFormatter_updatePageWhenCursorMoved
    ( PageFormatter * o,
      uint32_t moveFlags,
      LPM_SelectionCursor * txtCurs )
{
    _resetAllLineChangedFlags(o);
    _resetAddChars(o);

    DspCurs dspCurs;
    _copyDisplayCursor(&dspCurs, &o->displayCursor);

    PageStatus pageStatus = _moveFlagsToDisplayCursor(o, moveFlags, &dspCurs);

    _changePageByStatus(o, pageStatus);

    if(pageStatus == PAGE_CURR)
        _updateLineChangedFlagsByDisplayCursor(o, &dspCurs);
    _copyDisplayCursor(&o->displayCursor, &dspCurs);

    _displayCursorToTextCursor(o, txtCurs);
}

void PageFormatter_updateWholePage(PageFormatter * o)
{
    _setAllLineChangedFlags(o);
    PageFormatter_updateDisplay(o);
}

void PageFormatter_updateDisplay
        ( PageFormatter * o )
{
    const LineMap * lineMap = o->pageStruct.lineMapTable;
    const LineMap * end     = lineMap + o->pageParams->lineAmount;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    size_t lineIndex  = 0;
    for( ; lineMap != end; lineBase += lineMap->fullLen, lineMap++, lineIndex++)
        if(_readLineChangedFlag(o, lineIndex))
            _displayLine(o, lineIndex, lineBase);
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

bool PageFormatter_fillBuffWithAddChars
    ( PageFormatter * o,
      Unicode_Buf * buf,
      LPM_EndlType endlType )
{    
    if(buf->size < PageFormatter_addCharsAmount(o, endlType))
        return false;

    unicode_t * pchr = buf->data;
    unicode_t * pend;

    if(endlType == LPM_ENDL_TYPE_CRLF)
    {
        pend = pchr + o->addChars.lines * 2;
        for( ; pchr != pend; pchr++)
        {
            *pchr++ = chrCr;
            *pchr   = chrLf;
        }
    }

    else if(endlType == LPM_ENDL_TYPE_CR)
    {
        pend = pchr + o->addChars.lines;
        for( ; pchr != pend; pchr++)
            *pchr = chrCr;
    }

    else if(endlType == LPM_ENDL_TYPE_LF)
    {
        pend = pchr + o->addChars.lines;
        for( ; pchr != pend; pchr++)
            *pchr = chrLf;
    }

    else
    {
        return false;
    }

    pend += o->addChars.spaces;
    for( ; pchr != pend; pchr++)
        *pchr = chrSpace;

    buf->size = pend - buf->data;
    return true;
}


size_t PageFormatter_fillPageWithEmptyLines
        ( PageFormatter * o,
          Unicode_Buf * buf,
          LPM_EndlType endlType )
{
    buf->data = o->modules->lineBuffer.data;
    buf->size = o->pageParams->lineAmount;
    size_t offset = 0;

    if(!o->pageStruct.prevLastLine.endsWithEndl)
    {
        buf->size++;
        offset = 1;
    }

    unicode_t * pchr = buf->data;
    unicode_t * pend = pchr + buf->size;

    if(endlType == LPM_ENDL_TYPE_CRLF)
    {
        pend += buf->size;
        buf->size *= 2;
        offset *= 2;
        for( ; pchr != pend; pchr++)
        {
            *pchr++ = chrCr;
            *pchr   = chrLf;
        }
    }

    else if(endlType == LPM_ENDL_TYPE_CR)
    {
        for( ; pchr != pend; pchr++)
            *pchr = chrCr;
    }

    else if(endlType == LPM_ENDL_TYPE_LF)
    {
        for( ; pchr != pend; pchr++)
            *pchr = chrLf;
    }

    else
    {
        buf->size = 0;
    }

    return offset;
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
}

void _selectAllPage(Obj * o, DspCurs * dspCurs)
{
    o->selectBackward = false;
    _setDspPosToPageBegin(o, &dspCurs->begin);
    _setDspPosToPageEnd(o, &dspCurs->end);
    _incDspXPos(o, &dspCurs->end);
}

void _selectAllLine(Obj * o, DspCurs * dspCurs)
{
    o->selectBackward = false;
    _setDspPosToLineBegin(o, &dspCurs->begin);
    _setDspPosToLineEnd(o, &dspCurs->end);
    _incDspXPos(o, &dspCurs->end);
    return;
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

void _displayCursorToTextCursor(Obj * o, SlcCurs * textCursor)
{
    size_t cursBegin = textCursor->pos;
    size_t cursEnd = cursBegin + textCursor->len;

    const LineMap * lm = o->pageStruct.lineMapTable;
    const LineMap * const end = lm + o->pageParams->lineAmount;
    size_t lineIndex = 0;
    size_t lineBase = _calcCurrPageFirstLineBase(o);
    size_t firstEmptyLineIndex = o->pageParams->lineAmount;
    for( ; lm < end; lineBase += lm->fullLen, lm++, lineIndex++)
    {
        if(lineIndex == o->displayCursor.begin.y)
        {
            cursBegin = _calcTextPosByDisplayXPos(o, lineBase, lm, o->displayCursor.begin.x);
            _calcAddSpaceAmount(o, lm, o->displayCursor.begin.x);
        }

        if(lineIndex == o->displayCursor.end.y)
        {
            cursEnd = _calcTextPosByDisplayXPos(o, lineBase, lm, o->displayCursor.end.x);
        }

        if(lm->fullLen == 0)
            if(firstEmptyLineIndex == o->pageParams->lineAmount)
                firstEmptyLineIndex = lineIndex;
    }
    textCursor->pos = cursBegin;
    textCursor->len = cursEnd - cursBegin;
    _calcAddLineAmount(o, firstEmptyLineIndex);
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

void _resetAddChars(Obj * o)
{
    o->addChars.spaces = 0;
    o->addChars.lines = 0;
}

void _calcAddSpaceAmount(Obj * o, const LineMap * lm, size_t xPos)
{
    o->addChars.spaces = _dspXPosIsAfterLinePayloadPart(o, lm, xPos) ?
                lm->restLen + xPos - o->pageParams->charAmount : 0;
}

void _calcAddLineAmount(Obj * o, size_t firstEmptyLineIndex)
{
    if(firstEmptyLineIndex == o->pageParams->lineAmount)
        return;

    const LineMap * prevLine = (firstEmptyLineIndex == 0) ?
                (&o->pageStruct.prevLastLine) :
                (o->pageStruct.lineMapTable + firstEmptyLineIndex-1);

    if(o->displayCursor.begin.y >= firstEmptyLineIndex)
    {
        if(!prevLine->endsWithEndl)
            firstEmptyLineIndex--;
        o->addChars.lines = o->displayCursor.begin.y - firstEmptyLineIndex;
    }
//    if(prevLine->endsWithEndl)
//    {
//        if(o->displayCursor.begin.y >= firstEmptyLineIndex)
//        {
//            o->addChars.lines = o->displayCursor.begin.y - firstEmptyLineIndex;
//        }
//    }
//    else
//    {
//        if(o->displayCursor.begin.y >= firstEmptyLineIndex)
//        {
//            o->addChars.lines = o->displayCursor.begin.y - firstEmptyLineIndex+1;
//        }
//    }
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

void _displayLine(Obj * o, size_t lineIndex, size_t lineOffset)
{
    Unicode_Buf lineBuf;
    const LineMap * lineMap = o->pageStruct.lineMapTable + lineIndex;
    SlcCurs lineCursor;
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
