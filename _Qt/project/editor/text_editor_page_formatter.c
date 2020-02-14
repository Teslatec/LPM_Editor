#include "text_editor_page_formatter.h"
#include "text_editor_text_operator.h"

#include <string.h>

#define LINE_AMOUNT TEXT_EDITOR_PAGE_LINE_AMOUNT
#define CHAR_AMOUNT TEXT_EDITOR_PAGE_CHAR_AMOUNT
#define LAST_GROUP     (TEXT_EDITOR_PAGE_GROUP_AMOUNT-1)
#define PAGES_IN_GROUP TEXT_EDITOR_PAGES_IN_GROUP

typedef TextEditorLineMap       LineMap;
typedef TextEditorPageFormatter Obj;
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


static void _buildFirstPage(Obj * o);
static void _buildNextPage(Obj * o);
static void _buildPreviousPage(Obj * o);
static void _rebuildCurrentPage(Obj * o, const SlcCurs * changedTextArea);

static TextPos _checkTextPos(Obj * o, size_t pos);
static bool _isOnFirstPage(Obj * o);
static bool _isOnLastPage(Obj * o);

static size_t _calcCurrPageLen(Obj * o);

static void _buildPageFromCurrPageOffset(Obj * o);
static void _buildPageBackwardFromCurrentPosition(Obj * o);
static void _switchToNextPage(Obj * o);

static bool _incPageIndex(Obj * o);
static void _decPageIndex(Obj * o);
static void _resetPageIndexToGroupBegin(Obj * o);
static void _resetPageIndexToTextBegin(Obj * o);
static bool _isCurrentPageFirst(Obj * o);

static void _resetPrevPageLastLineMap(Obj * o);
static void _writePrevPageLastLineMap(Obj * o, const LineMap * lineMap);

static size_t _buildLineMapForward(Obj * o, LineMap * lineMap);
static size_t _rebuildLineMapAndCheckItChanged(Obj * o, LineMap * lineMap);
static size_t _buildLineMapBackward(Obj * o, LineMap * lineMap, size_t lineOffset);
static void _convertLineTableToForwardRepresentation(Obj * o);

static bool _checkForEndOfText(unicode_t chr);
static bool _checkForEndLine(unicode_t chr);
static bool _checkForDiacritic(unicode_t chr);
static bool _checkForWordDivider(unicode_t chr);

static size_t _buildLineMapWithEndOfText(LineMap * lineMap, size_t currPos, size_t placeCnt);
static size_t _buildLineMapWithEndLine(LineMap * lineMap, const unicode_t * pchr, size_t currPos, size_t placeCnt);
static size_t _buildLineMapWithWordDividerAtLineEnd(LineMap * lineMap, const unicode_t * pchr, size_t currPos);
static size_t _buildLineMapWithVeryLongWord(LineMap * lineMap, const unicode_t * pchr, size_t currPos);
static size_t _buildLineMapWithWordWrap(LineMap * lineMap, size_t lastWordDivPos, size_t placeCntAtLastWordDiv);

static void _loadLineForward(Obj * o, size_t loadPos);
static void _loadLineBackward(Obj * o, size_t lineBackwardOffset);

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


static void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset);


void TextEditorPageFormatter_init
        ( TextEditorPageFormatter * o,
          const TextEditorModules * modules )
{
    memset(o, 0, sizeof(TextEditorPageFormatter));
    o->modules = modules;
}

void TextEditorPageFormatter_setPageAtTextPosition
        ( TextEditorPageFormatter * o,
          size_t pos )
{
    _buildFirstPage(o);
    while(_checkTextPos(o, pos) != TEXT_POS_ON_CURRENT_PAGE)
        _buildNextPage(o);
}

void TextEditorPageFormatter_updatePageByTextChanging
        ( TextEditorPageFormatter * o,
          const LPM_SelectionCursor * changedTextArea,
          size_t newTextPosition )
{
    _rebuildCurrentPage(o, changedTextArea);
    TextPos tp = _checkTextPos(o, newTextPosition);
    if(tp == TEXT_POS_BEFORE_CURRENT_PAGE)
        _buildPreviousPage(o);
    else if(tp == TEXT_POS_AFTER_CURRENT_PAGE)
        _buildNextPage(o);
}

void TextEditorPageFormatter_updateDisplay
        ( TextEditorPageFormatter * o )
{
    LPM_Point pos = { .x = 0, .y = 0 };
    Unicode_Buf lineBuf = { .data = o->modules->lineBuffer.data, .size = 0 };

    LPM_UnicodeDisplay_setCursor(o->modules->display, &o->displayCursor);

    const LineMap * lineMap = o->lineMap;
    const LineMap * end     = o->lineMap + LINE_AMOUNT;
    size_t lineOffset = o->currPageOffset;
    size_t lineIndex  = 0;
    for( ; lineMap != end; lineMap++, lineIndex++, pos.y++)
    {
        if(_readLineChangedFlag(o, lineIndex))
        {
            _formatLineForDisplay(o, lineMap, lineOffset);
            lineBuf.size = lineMap->payloadLen + lineMap->restLen;
            LPM_UnicodeDisplay_writeLine(o->modules->display, &lineBuf, &pos);
        }
        lineOffset += lineMap->fullLen;
    }
}

void _buildFirstPage(Obj * o)
{
    _resetAllLineChangedFlags(o);
    _resetPageIndexToTextBegin(o);
    o->pageGroupOffset[0] = 0;
    _resetPrevPageLastLineMap(o);
    _buildPageFromCurrPageOffset(o);
}

void _buildNextPage(Obj * o)
{
    _resetAllLineChangedFlags(o);
    if(_isOnLastPage(o))
        return;
    _writePrevPageLastLineMap(o, o->lineMap+LINE_AMOUNT-1);
    _switchToNextPage(o);
    _buildPageFromCurrPageOffset(o);
//    printf("page+: %d group: %d offset table: %d %d %d %d\n",
//           o->currPageIndex,
//           o->currGroupIndex,
//           o->pageGroupOffset[0],
//           o->pageGroupOffset[1],
//           o->pageGroupOffset[2],
//           o->pageGroupOffset[3] );
}

void _buildPreviousPage(Obj * o)
{
    _resetAllLineChangedFlags(o);
    if(_isOnFirstPage(o))
        return;

    _decPageIndex(o);
    size_t goalPageIndex = o->currPageIndex;
    _resetPageIndexToGroupBegin(o);

    o->currPageOffset = o->pageGroupOffset[o->currGroupIndex];
    _buildPageFromCurrPageOffset(o);
    for(;;)
    {
        if(o->currPageIndex == goalPageIndex)
            break;

        _buildNextPage(o);
    }
//    printf("page-: %d group: %d offset table: %d %d %d %d\n",
//           o->currPageIndex,
//           o->currGroupIndex,
//           o->pageGroupOffset[0],
//           o->pageGroupOffset[1],
//           o->pageGroupOffset[2],
//           o->pageGroupOffset[3] );
}

void _rebuildCurrentPage(Obj * o, const SlcCurs * changedTextArea)
{
    _resetAllLineChangedFlags(o);
    o->lastPageReached = false;

    size_t lineEndPosBeforeLineBuild = o->currPageOffset;
    size_t lineEndPosAfterLineBuild = o->currPageOffset;

    if(!_isCurrentPageFirst(o))
    {
        size_t prevLineLen = o->prevPageLastLineMap.fullLen;
        _loadLineForward(o, o->currPageOffset - prevLineLen);

        lineEndPosBeforeLineBuild = o->currPageOffset;
        _buildLineMapForward(o, &o->prevPageLastLineMap);
        int diff = (int)(o->prevPageLastLineMap.fullLen) - (int)prevLineLen;
        o->currPageOffset += diff;
        lineEndPosAfterLineBuild = o->currPageOffset;
    }

    size_t lineOffset = 0;
    size_t lineIndex = 0;
    LineMap * lineMap = o->lineMap;
    LineMap * end     = o->lineMap + LINE_AMOUNT;
    for( ; lineMap != end; lineMap++, lineIndex++)
    {
        if(lineEndPosBeforeLineBuild != lineEndPosAfterLineBuild)
            _setLineChangedFlag(o, lineIndex);

        _loadLineForward(o, o->currPageOffset + lineOffset);

        lineEndPosBeforeLineBuild = o->currPageOffset + lineOffset + lineMap->fullLen;
        size_t lineOffsetInc = _buildLineMapForward(o, lineMap);
        lineEndPosAfterLineBuild = o->currPageOffset + lineOffset + lineMap->fullLen;

        if(lineEndPosBeforeLineBuild != lineEndPosAfterLineBuild)
            _setLineChangedFlag(o, lineIndex);

        if(_changedTextCrossesLine( changedTextArea,
                                    o->currPageOffset + lineOffset,
                                    lineMap->fullLen ))
            _setLineChangedFlag(o, lineIndex);

        lineOffset += lineOffsetInc;
    }
}

TextPos _checkTextPos(Obj * o, size_t pos)
{
    size_t currPageBegin = o->currPageOffset;
    size_t currPageEnd = currPageBegin + _calcCurrPageLen(o);

    if(_isOnFirstPage(o))
        return pos < currPageEnd ? TEXT_POS_ON_CURRENT_PAGE : TEXT_POS_AFTER_CURRENT_PAGE;

    if(_isOnLastPage(o))
        return pos >= currPageBegin ? TEXT_POS_ON_CURRENT_PAGE : TEXT_POS_BEFORE_CURRENT_PAGE;

    if(pos < currPageBegin)
        return TEXT_POS_BEFORE_CURRENT_PAGE;
    if(pos >= currPageEnd)
        return TEXT_POS_AFTER_CURRENT_PAGE;
    return TEXT_POS_ON_CURRENT_PAGE;
}

bool _isOnFirstPage(Obj * o)
{
    return (o->currPageIndex == 0) && (o->currGroupIndex == 0);
}

bool _isOnLastPage(Obj * o)
{
    return o->lastPageReached;
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

void _buildPageFromCurrPageOffset(Obj * o)
{
    _setAllLineChangedFlags(o);
    o->lastPageReached = false;
    size_t lineOffset = 0;
    LineMap * lineMap = o->lineMap;
    LineMap * end     = o->lineMap + LINE_AMOUNT;
    for( ; lineMap != end; lineMap++)
    {
        _loadLineForward(o, o->currPageOffset + lineOffset);
        lineOffset += _buildLineMapForward(o, lineMap);
    }
}

void _buildPageBackwardFromCurrentPosition(Obj * o)
{
    _setAllLineChangedFlags(o);
    size_t lineOffset = 0;
    LineMap * lineMap = o->lineMap + LINE_AMOUNT-1;
    LineMap * end     = o->lineMap-1;
    for( ; lineMap != end; lineMap--)
        lineOffset += _buildLineMapBackward(o, lineMap, lineOffset);
    o->currPageOffset -= lineOffset;
    _convertLineTableToForwardRepresentation(o);
}

void _switchToNextPage(Obj * o)
{
    const LineMap * lineMap = o->lineMap;
    const LineMap * end     = o->lineMap + LINE_AMOUNT;
    for( ; lineMap != end; lineMap++)
        o->currPageOffset += lineMap->fullLen;

    if(_incPageIndex(o))
        o->pageGroupOffset[o->currGroupIndex] = o->currPageOffset;
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

void _decPageIndex(Obj * o)
{
    if(o->currPageIndex == 0)
    {
        if(o->currGroupIndex > 0)
        {
            o->currGroupIndex--;
            o->currPageIndex = PAGES_IN_GROUP-1;
        }
    }
    else
    {
        o->currPageIndex--;
    }
}

void _resetPageIndexToGroupBegin(Obj * o)
{
    o->currPageIndex = 0;
}

void _resetPageIndexToTextBegin(Obj * o)
{
    o->currPageOffset = 0;
    o->currGroupIndex = 0;
    o->currPageIndex  = 0;
}

static bool _isCurrentPageFirst(Obj * o)
{
    return (o->currPageIndex == 0) && (o->currGroupIndex == 0);
}

void _resetPrevPageLastLineMap(Obj * o)
{
    o->prevPageLastLineMap.fullLen    = 0;
    o->prevPageLastLineMap.payloadLen = 0;
    o->prevPageLastLineMap.restLen    = 0;
}

void _writePrevPageLastLineMap(Obj * o, const LineMap * lineMap)
{
    o->prevPageLastLineMap.fullLen    = lineMap->fullLen;
    o->prevPageLastLineMap.payloadLen = lineMap->payloadLen;
    o->prevPageLastLineMap.restLen    = lineMap->restLen;
}

size_t _buildLineMapForward(Obj * o, LineMap * lineMap)
{
    const size_t maxPlaceCntValue = CHAR_AMOUNT;
    const size_t wordDividersNotFoundValue = o->modules->lineBuffer.size;

    size_t currPos = 0;
    size_t placeCnt = 0;
    size_t lastWordDivPos = wordDividersNotFoundValue;
    size_t placeCntAtLastWordDiv = 0;

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

        placeCnt++;
    }

    if(_checkForWordDivider(*pchr))
        return _buildLineMapWithWordDividerAtLineEnd(lineMap, pchr, currPos);

    if(lastWordDivPos == wordDividersNotFoundValue)
        return _buildLineMapWithVeryLongWord(lineMap, pchr, currPos);

    return _buildLineMapWithWordWrap(lineMap, lastWordDivPos, placeCntAtLastWordDiv);
}

size_t _buildLineMapBackward(Obj * o, LineMap * lineMap, size_t lineOffset)
{
    _loadLineBackward(o, lineOffset);

    const size_t maxPlaceCntValue = CHAR_AMOUNT;
    const size_t wordDividersNotFoundValue = o->modules->lineBuffer.size;

    size_t currPos = 0;
    size_t placeCnt = 0;
    size_t lastWordDivPos = 0;
    size_t placeCntAtLastWordDiv = 0;

    const unicode_t * pend = o->modules->lineBuffer.data-1;
    const unicode_t * pchr = pend + o->modules->lineBuffer.size;

    lineMap->fullLen    = 0;

    // Проверяем конец строки
    if(*pchr == chrLf)
    {
        lineMap->fullLen++;
        pchr--;
    }
    if(*pchr == chrCr)
    {
        lineMap->fullLen++;
        pchr--;
    }
    if(*pchr == chrSpace)
    {
        lineMap->fullLen++;
        pchr--;
    }

    for( ; pchr != pend; pchr--, currPos++ )
    {
        if(_checkForEndOfText(*pchr))
        {
            lineMap->fullLen    += currPos;
            lineMap->payloadLen  = currPos;
            lineMap->restLen     = CHAR_AMOUNT - placeCnt;
        }

        if(_checkForEndLine(*pchr))
        {
            lineMap->fullLen += currPos;
            lineMap->payloadLen = currPos;
            lineMap->restLen = CHAR_AMOUNT - placeCnt;
            return lineMap->fullLen;
        }

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

        placeCnt++;
    }

    if((*pchr == chrSpace) || (*pchr == chrLf) || (*pchr == chrCr))
    {
        lineMap->fullLen += currPos;
        lineMap->payloadLen = currPos;
        lineMap->restLen = 0;
        return lineMap->fullLen;
    }

    if(lastWordDivPos == wordDividersNotFoundValue)
    {
        lineMap->fullLen += currPos;
        lineMap->restLen = 0;
        lineMap->payloadLen = currPos;
        return lineMap->fullLen;
    }


    lineMap->payloadLen = lastWordDivPos;
    lineMap->restLen = CHAR_AMOUNT - placeCntAtLastWordDiv;
    lineMap->fullLen += lastWordDivPos+1;
    return lineMap->fullLen;
}

void _convertLineTableToForwardRepresentation(Obj * o)
{}

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

void _loadLineForward(Obj * o, size_t loadPos)
{
    Unicode_Buf buf =
    {
        o->modules->lineBuffer.data,
        o->modules->lineBuffer.size
    };
    size_t fullSize = o->modules->lineBuffer.size;
    TextEditorTextOperator_read(o->modules->textOperator, loadPos, &buf);
    memset(buf.data + buf.size, 0, (fullSize - buf.size)*sizeof(unicode_t));
}

void _loadLineBackward(Obj * o, size_t lineBackwardOffset)
{
    const size_t lineBufSize = o->modules->lineBuffer.size;
    unicode_t * const lineBufData = o->modules->lineBuffer.data;

    memset(lineBufData, 0, lineBufSize * sizeof(unicode_t));

    if(lineBackwardOffset > o->currPageOffset)
        return;

    const size_t loadEndPos = o->currPageOffset - lineBackwardOffset;
    Unicode_Buf buf;
    size_t loadStartPos;

    if(loadEndPos < lineBufSize)
    {
        loadStartPos = 0;
        buf.data = lineBufData + (loadEndPos - lineBufSize);
        buf.size = loadEndPos;
    }
    else
    {
        loadStartPos = loadEndPos-lineBufSize;
        buf.data = lineBufData;
        buf.size = lineBufSize;
    }

    TextEditorTextOperator_read(o->modules->textOperator, loadStartPos, &buf);
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

void _formatLineForDisplay(Obj * o, const LineMap * lineMap, size_t lineOffset)
{
    Unicode_Buf buf =
    {
        .data = o->modules->lineBuffer.data,
        .size = lineMap->payloadLen,
    };
    TextEditorTextOperator_read(o->modules->textOperator, lineOffset, &buf);

    unicode_t * pchr = buf.data + lineMap->payloadLen;
    unicode_t * const end = pchr + lineMap->restLen;
    for( ; pchr != end; pchr++)
        *pchr = chrSpace;
}
