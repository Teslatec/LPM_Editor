#include "page_formatter.h"
#include "lpm_text_storage.h"
#include "lpm_text_operator.h"
#include "crc16_table.h"
#include "editor_flags.h"

#include <string.h>

#define LINE_AMOUNT PAGE_LINE_AMOUNT
#define CHAR_AMOUNT PAGE_CHAR_AMOUNT
#define LAST_GROUP  (PAGE_GROUP_AMOUNT-1)

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

static void _updateDisplayCursorByTextCursor(Obj * o, const SlcCurs * curs);
static size_t _calcDisplayCursorByLineMapAnalizing(Obj * o, const SlcCurs * txtCurs, DspCurs * dspCurs);
static void _setDisplayCursorValueForEmptyPage(DspCurs * curs);
static void _setDisplayCursorInvalidValue(DspCurs * curs);
static bool _isDisplayCursorPointInvalid(const LPM_Point * point);
static void _validateDisplayCursorPointIfInvalid(Obj * o, size_t lineIndex, LPM_Point * point);

static void _displayCursorToLineCursor(Obj * o, size_t lineIndex, size_t lineSize, SlcCurs * lineCursor);

static bool _moveFlagsToTextCursor(Obj * o, uint32_t moveFlags, SlcCurs * textCurs);
static void _moveCursorToLineBorder(Obj * o, uint32_t borderFlag, SlcCurs * textCurs);
static bool _moveCursorToPageBorder(Obj * o, uint32_t borderFlag, SlcCurs * textCurs);
static void _moveCursorAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs);

static void _selectAllPage(Obj * o, SlcCurs * textCurs);
static void _selectAllLine(Obj * o, SlcCurs * textCurs);
static void _changeSelectionAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs);

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
static void _calcSymmetricDifference
        ( const SlcCurs * src1,
          const SlcCurs * src2,
          SlcCurs * dst1,
          SlcCurs * dst2 );
static void _normalizeTextCursorAndFillBeginEnd(const SlcCurs * curs, size_t * begin, size_t * end);
static void _updateLineChangedFlagsByTextCursorChanging
        (Obj * o, const SlcCurs * symDiff1, const SlcCurs * symmDiff2);


static void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset);

void test_beep();

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
    _changePageIfNotOnCurrTextPosition(o, curs->pos);
    _updateDisplayCursorByTextCursor(o, curs);
    _saveTextCursor(o, curs);
}

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs )
{
    _resetAllLineChangedFlags(o);
    //_setAllLineChangedFlags(o);
    _updateLinesMap(o);
    _updatePageByTextCursor(o, curs);
}


void PageFormatter_updatePageWhenCursorMoved
    ( PageFormatter * o,
      uint32_t moveFlags,
      LPM_SelectionCursor * textCurs )
{
    _resetAllLineChangedFlags(o);
    //_setAllLineChangedFlags(o);
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
    LineMap * const end = o->pageStruct.lineMapTable + LINE_AMOUNT;
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
            _loadLine(o, lineBase, o->modules->lineBuffer.size);

    LPM_TextLineMap textLineMap;
    if(LPM_TextOperator_analizeLine( o->modules->textOperator,
                                     begin,
                                     CHAR_AMOUNT,
                                     &textLineMap) )
        endOfTextFind = true;

    lineMap->fullLen    = (uint8_t)(textLineMap.nextLine    - begin);
    lineMap->payloadLen = (uint8_t)(textLineMap.printBorder - begin);
    lineMap->restLen    = CHAR_AMOUNT - textLineMap.lenInChr;
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

unicode_t * _loadLine(Obj * o, size_t loadPos, size_t loadSize)
{
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        loadSize
    };
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
        _setDisplayCursorInvalidValue(&o->displayCursor);
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
    const LineMap * const end = lineMap + LINE_AMOUNT;
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

void _setDisplayCursorInvalidValue(DspCurs * curs)
{
    curs->begin.x = 0;
    curs->begin.y = LINE_AMOUNT;
    curs->end.x = 0;
    curs->end.y = LINE_AMOUNT;
}

bool _isDisplayCursorPointInvalid(const LPM_Point * point)
{
    return point->y == LINE_AMOUNT;
}

void _validateDisplayCursorPointIfInvalid(Obj * o, size_t lineIndex, LPM_Point * point)
{
    if(_isDisplayCursorPointInvalid(point))
    {
        const LineMap * lineMap = o->pageStruct.lineMapTable + lineIndex;
        if(lineIndex == LINE_AMOUNT-1)
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
    textCurs->len = 0;
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
    textCurs->len = 0;
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
        _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
    }
    else // CURSOR_FLAG_PREV
    {
        if(_isCurrPageFirst(o))
            return false;
        textCurs->pos = o->pageStruct.base;
        _changePageIfNotOnCurrTextPosition(o, textCurs->pos);
        textCurs->pos = _calcCurrPageFirstLineBase(o);
    }

    _updateDisplayCursorByTextCursor(o, textCurs);
    return true;
}

void _moveCursorAtChar(Obj * o, uint32_t dirFlags, SlcCurs * textCurs)
{
    if(dirFlags == CURSOR_FLAG_LEFT)
    {
        if(textCurs->len > 0)
            textCurs->len = 0;
        else
            textCurs->pos = _movePosLeftAtChar(o, textCurs->pos);
    }
    else if(dirFlags == CURSOR_FLAG_RIGHT)
    {
        if(textCurs->len > 0)
        {
            textCurs->pos += textCurs->len;
            textCurs->len = 0;
        }
        else
            textCurs->pos = _movePosRightAtChar(o, textCurs->pos);
    }
    else if(dirFlags == CURSOR_FLAG_UP)
    {
        if(textCurs->len > 0)
            textCurs->len = 0;
        else
        {
            textCurs->pos = _movePosUpAtChar(o, textCurs->pos, &o->displayCursor.begin);
        }
    }
    else //if(dirFlags == CURSOR_FLAG_DOWN)
    {
        if(textCurs->len > 0)
        {
            textCurs->pos += textCurs->len;
            textCurs->len = 0;
        }
        else
        {
            textCurs->pos = _movePosDownAtChar(o, textCurs->pos, &o->displayCursor.begin);
        }
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
    size_t begin;
    size_t end;
    const LPM_Point * dspPoint;

    if(o->selectBackward)
    {
        end = textCurs->pos;
        begin = end + textCurs->len;
        dspPoint = &o->displayCursor.begin;
    }
    else
    {
        begin = textCurs->pos;
        end = begin + textCurs->len;
        dspPoint = &o->displayCursor.end;
    }

    if(dirFlags == CURSOR_FLAG_LEFT)
    {
        end = _movePosLeftAtChar(o, end);
    }
    else if(dirFlags == CURSOR_FLAG_RIGHT)
    {
        end = _movePosRightAtChar(o, end);
    }
    else if(dirFlags == CURSOR_FLAG_UP)
    {
        end = _movePosUpAtChar(o, end, dspPoint);
    }
    else //if(dirFlags == CURSOR_FLAG_DOWN)
    {
        end = _movePosDownAtChar(o, end, dspPoint);
    }

    size_t pageBegin = _calcCurrPageFirstLineBase(o);
    size_t pageEnd = pageBegin + _calcCurrPageLen(o);
    if(end < pageBegin)
        end = pageBegin;
    if(end >= pageEnd)
        end = pageEnd;

    if(o->selectBackward)
    {
        if(begin <= end)
        {
            o->selectBackward = false;
            textCurs->pos = begin;
            textCurs->len = end - begin;
        }
        else
        {
            textCurs->pos = end;
            textCurs->len = begin - end;
        }
    }
    else
    {
        if(end < begin)
        {
            o->selectBackward = true;
            textCurs->pos = end;
            textCurs->len = begin - end;
        }
        else
        {
            textCurs->pos = begin;
            textCurs->len = end - begin;
        }
    }
}

size_t _movePosLeftAtChar(Obj * o, size_t pos)
{
    if(pos > 0)
    {
        size_t loadLen = pos < 10 ? pos : 10;
        unicode_t * pchr = _loadLine(o, pos-loadLen, loadLen) + loadLen;
        pos -= pchr - LPM_TextOperator_prevChar(o->modules->textOperator, pchr);
    }
    return pos;
}

size_t _movePosRightAtChar(Obj * o, size_t pos)
{
    unicode_t * pchr = _loadLine(o, pos, 10);
    pos += LPM_TextOperator_nextChar(o->modules->textOperator, pchr) - pchr;
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
            const unicode_t * pchr = _loadLine(o, textPos, o->modules->lineBuffer.size);
            size_t chrAmount = LPM_TextOperator_calcChrAmount(o->modules->textOperator, pchr, pchr+currLineX);
            textPos -= prevLine->fullLen;
            pchr = _loadLine(o, textPos, o->modules->lineBuffer.size);
            textPos += LPM_TextOperator_nextNChar(o->modules->textOperator, pchr, chrAmount) - pchr;
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

    if(lineIndex == LINE_AMOUNT-1)
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
        const unicode_t * pchr = _loadLine(o, textPos, o->modules->lineBuffer.size);
        size_t chrAmount = LPM_TextOperator_calcChrAmount(o->modules->textOperator, pchr, pchr + currLineX);
        textPos += currLine->fullLen;
        pchr = _loadLine(o, textPos, o->modules->lineBuffer.size);
        textPos += LPM_TextOperator_nextNChar(o->modules->textOperator, pchr, chrAmount) - pchr;
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
    const LineMap * const end = lineMap + LINE_AMOUNT;
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
