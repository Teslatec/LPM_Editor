#include "lpm_text_operator.h"
#include "lpm_lang.h"

static const unicode_t chrEndOfText = 0x0000;
static const unicode_t chrCr = 0x000D;
static const unicode_t chrLf = 0x000A;
static const unicode_t chrSpace = 0x0020;

static bool _charBelongsToBasicLatin(unicode_t chr);
static bool _atEndOfLine(unicode_t chr);

static const unicode_t * _nextCharWhenEndOfLine(const unicode_t * pchr);
static const unicode_t * _prevCharWhenEndOfLine(const unicode_t * pchr);

bool LPM_TextOperator_checkInputChar
    ( LPM_TextOperator * o,
      unicode_t inputChr,
      const unicode_t * pchr )
{
    if(_charBelongsToBasicLatin(inputChr))
        return true;

    if( (inputChr == chrCr) ||
        (inputChr == chrLf) ||
        (inputChr == chrEndOfText) )
        return true;

    if(!LPM_Lang_isCharBelongsToLang(o->lang, &inputChr))
        return false;

    return LPM_Lang_checkInputChar(o->lang, inputChr, pchr);
}

const unicode_t * LPM_TextOperator_nextChar
    ( LPM_TextOperator * o,
      const unicode_t * pchr )
{
    if(*pchr == chrEndOfText)
        return pchr;

    if(_atEndOfLine(*pchr))
        return _nextCharWhenEndOfLine(pchr);

    return LPM_Lang_nextChar(o->lang, pchr);
}

const unicode_t * LPM_TextOperator_prevChar
    ( LPM_TextOperator * o,
      const unicode_t * pchr )
{
    if(_atEndOfLine(*pchr))
        return _prevCharWhenEndOfLine(pchr);

    return LPM_Lang_prevChar(o->lang, pchr);
}

bool LPM_TextOperator_atEndOfText(LPM_TextOperator * o, const unicode_t * pchr)
{
    (void)o;
    return *pchr == chrEndOfText;
}

bool LPM_TextOperator_atEndOfLine(LPM_TextOperator * o, const unicode_t * pchr)
{
    (void)o;
    return _atEndOfLine(*pchr);
}

bool LPM_TextOperator_atSpace(LPM_TextOperator * o, const unicode_t * pchr)
{
    (void)o;
    return *pchr == chrSpace;
}

bool _charBelongsToBasicLatin(unicode_t chr)
{
    return ((chr >= 0x0020) && (chr <= 0x007E));
}

bool _atEndOfLine(unicode_t chr)
{
    return (chr == chrCr) || (chr == chrLf);
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
    if(*pchr == chrCr)
        --pchr;
    return pchr;
}
