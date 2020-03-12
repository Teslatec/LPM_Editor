#include "text_operator.h"
#include "lpm_lang_api.h"

static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrSpace = 0x0020;

static bool _charBelongsToBasicLatin(unicode_t chr);
static bool _atEndOfLine(unicode_t chr);
static bool _atEndOfText(unicode_t chr);
static bool _atSpace(unicode_t chr);

static const unicode_t * _nextCharWhenEndOfLine(const unicode_t * pchr);
static const unicode_t * _prevCharWhenEndOfLine(const unicode_t * pchr);

static size_t _calcChrAmountForward(TextOperator * o, const unicode_t * begin, const unicode_t * end);
static size_t _calcChrAmountBackward(TextOperator * o, const unicode_t * begin, const unicode_t * end);

static void _fillLineMapWhenAtEndOfText
        ( const unicode_t * pchr,
          size_t chrCnt,
          LPM_TextLineMap * lineMap );

static void _fillLineMapWhenAtEndOfLine
        ( const unicode_t * pchr,
          size_t chrCnt,
          LPM_TextLineMap * lineMap );

static void _fillLineMapWhenVeryLongWord
        ( const unicode_t * pchr,
          size_t chrCnt,
          LPM_TextLineMap * lineMap );

static void _fillLineMapWhenWordWrapped
        ( const unicode_t * pdiv,
          size_t divCnt,
          LPM_TextLineMap * lineMap );

bool TextOperator_checkInputChar
    ( TextOperator * o,
      unicode_t inputChr,
      const unicode_t * pchr )
{
    if( _charBelongsToBasicLatin(inputChr) ||
            _atEndOfLine(inputChr) ||
            _atEndOfText(inputChr))
        return true;

    return LPM_Lang_checkInputChar(o->lang, inputChr, pchr);
}

const unicode_t * TextOperator_nextChar
    ( TextOperator * o,
      const unicode_t * pchr )
{
    if(_atEndOfText(*pchr))
        return pchr;

    if(_atEndOfLine(*pchr))
        return _nextCharWhenEndOfLine(pchr);

    return LPM_Lang_nextChar(o->lang, pchr);
}

const unicode_t * TextOperator_prevChar
    ( TextOperator * o,
      const unicode_t * pchr )
{
    --pchr;

    if(_atEndOfText(*pchr))
        return pchr;

    if(_atEndOfLine(*pchr))
        return _prevCharWhenEndOfLine(pchr);

    return LPM_Lang_prevChar(o->lang, pchr);
}

bool TextOperator_analizeLine
        ( TextOperator * o,
          const unicode_t  * pchr,
          size_t maxLenInChrs,
          LPM_TextLineMap * lineMap )
{
    bool endOfTextReached = false;
    lineMap->endsWithEndl = false;

    const unicode_t * pWordDiv = pchr + maxLenInChrs;
    size_t wordDivCnt = maxLenInChrs;
    size_t chrCnt;

    for(chrCnt = 0; chrCnt < maxLenInChrs; chrCnt++)
    {
        if(_atEndOfText(*pchr) || _atEndOfLine(*pchr))
            break;

        if(_atSpace(*pchr))
        {
            wordDivCnt = chrCnt;
            pWordDiv   = pchr;
            if(chrCnt == maxLenInChrs-1)
                break;
        }

        pchr = LPM_Lang_nextChar(o->lang, pchr);
    }

    if(_atEndOfText(*pchr))
    {
        _fillLineMapWhenAtEndOfText(pchr, chrCnt, lineMap);
        endOfTextReached = true;
    }

    else if(_atEndOfLine(*pchr) || _atSpace(*pchr))
        _fillLineMapWhenAtEndOfLine(pchr, chrCnt, lineMap);

    else if(wordDivCnt == maxLenInChrs)
        _fillLineMapWhenVeryLongWord(pchr, maxLenInChrs, lineMap);

    else
        _fillLineMapWhenWordWrapped(pWordDiv, wordDivCnt, lineMap);

    return endOfTextReached;
}

const unicode_t * TextOperator_nextNChar
    ( TextOperator * o,
      const unicode_t * pchr,
      size_t chrAmount )
{
    for(size_t i = 0; i < chrAmount; i++)
    {
        if(_atEndOfText(*pchr))
            break;

        if(_atEndOfLine(*pchr))
            pchr = _nextCharWhenEndOfLine(pchr);
        else
            pchr = LPM_Lang_nextChar(o->lang, pchr);
    }
    return pchr;
}

size_t TextOperator_calcChrAmount
    ( TextOperator * o,
      const unicode_t * begin,
      const unicode_t * end )
{
    if(begin == end)
        return 0;

    if(begin > end)
        return _calcChrAmountBackward(o, begin, end);

    return _calcChrAmountForward(o, begin, end);
}

bool TextOperator_atEndOfText(TextOperator * o, const unicode_t * pchr)
{
    (void)o;
    return *pchr == chrEndOfText;
}

bool TextOperator_atEndOfLine(TextOperator * o, const unicode_t * pchr)
{
    (void)o;
    return _atEndOfLine(*pchr);
}

bool TextOperator_atSpace(TextOperator * o, const unicode_t * pchr)
{
    (void)o;
    return *pchr == chrSpace;
}

bool _charBelongsToBasicLatin(unicode_t chr)
{
    return ((chr >= 0x0020) && (chr <= 0x007E));
}

/*
 * Выключил код для проверки принадлежности к спец.символам, поскольку
 *  вообще-то этим должен заниматься модуль поддержки языка, потому что
 *  заранее неизвестно, допустимы ли, например, знаки ударения и т.п.
 *  Можно конечно добавить как частный случай именно непечатные символы,
 *  но для этого придется создавать где-то в памяти массив таких символов.
 *  Проще уж, чтобы этим занимался внешний модуль, так и легче будет менять
 *  такие наборы.
 */
#if 0

bool _charBelongsToSpecChars(unicode_t chr, const Unicode_Buf * specChars)
{
    if(specChars->data == NULL)
        return false;

    const unicode_t * pchr = specChars->data;
    const unicode_t * const end = specChars->data + specChars->size;
    for( ; pchr != end; pchr++ )
        if(chr == *pchr)
            return true;
    return false;
}

#endif

bool _atEndOfLine(unicode_t chr)
{
    return (chr == chrCr) || (chr == chrLf);
}

bool _atEndOfText(unicode_t chr)
{
    return chr == chrEndOfText;
}

bool _atSpace(unicode_t chr)
{
    return chr == chrSpace;
}

const unicode_t * _nextCharWhenEndOfLine(const unicode_t * pchr)
{
    if(*pchr == chrCr)
        ++pchr;
    if(*pchr == chrLf)
        ++pchr;
    return pchr;
}

const unicode_t * _prevCharWhenEndOfLine(const unicode_t * pchr)
{
    if(*pchr == chrLf)
        --pchr;
    return pchr;
}

void _fillLineMapWhenAtEndOfText
        ( const unicode_t * pchr,
          size_t chrCnt,
          LPM_TextLineMap * lineMap )
{
    lineMap->printBorder = pchr;
    lineMap->nextLine    = pchr;
    lineMap->lenInChr    = chrCnt;
}

void _fillLineMapWhenAtEndOfLine
        ( const unicode_t * pchr,
          size_t chrCnt,
          LPM_TextLineMap * lineMap )
{
    lineMap->printBorder = pchr;
    lineMap->lenInChr = chrCnt;
    lineMap->nextLine = pchr;

    if(*pchr == chrSpace)
    {
        lineMap->nextLine++;
        ++pchr;
    }

    if(*pchr == chrCr)
    {
        lineMap->nextLine++;
        lineMap->endsWithEndl = true;
        ++pchr;
    }

    if(*pchr == chrLf)
    {
        lineMap->nextLine++;
        lineMap->endsWithEndl = true;
    }
}

void _fillLineMapWhenVeryLongWord
        ( const unicode_t * pchr,
          size_t chrCnt,
          LPM_TextLineMap * lineMap )
{
    lineMap->printBorder = pchr;
    lineMap->lenInChr    = chrCnt;
    lineMap->nextLine    = pchr;
}

void _fillLineMapWhenWordWrapped
        ( const unicode_t * pdiv,
          size_t divCnt,
          LPM_TextLineMap * lineMap )
{
    lineMap->printBorder = pdiv;
    lineMap->lenInChr    = divCnt;
    lineMap->nextLine    = pdiv+1;
}

size_t _calcChrAmountForward
    ( TextOperator * o,
      const unicode_t * begin,
      const unicode_t * end )
{
    size_t chrAmount = 0;
    while(begin < end)
    {
        if(_atEndOfText(*begin))
            break;

        if(_atEndOfLine(*begin))
            begin = _nextCharWhenEndOfLine(begin);
        else
            begin = LPM_Lang_nextChar(o->lang, begin);
        chrAmount++;
    }
    return chrAmount;
}

size_t _calcChrAmountBackward
    ( TextOperator * o,
      const unicode_t * begin,
      const unicode_t * end )
{
    size_t chrAmount = 0;
    while(begin > end)
    {
        --begin;
        if(_atEndOfLine(*begin))
            begin = _prevCharWhenEndOfLine(begin);
        else
            begin = LPM_Lang_prevChar(o->lang, begin);
        chrAmount++;
    }
    return chrAmount;
}
