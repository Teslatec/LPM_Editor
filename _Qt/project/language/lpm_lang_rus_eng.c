#include "lpm_lang_rus_eng.h"

static const unicode_t chrYoBig   = 0x0401;
static const unicode_t chrYoSmall = 0x0451;
static const unicode_t chrCratca  = 0x0306;
static const unicode_t chrUmlaut  = 0x0308;
static const unicode_t chrIBig    = 0x0418;
static const unicode_t chrISmall  = 0x0438;
static const unicode_t chrEBig    = 0x0415;
static const unicode_t chrESmall  = 0x0435;

static bool _withinBaseRussian(unicode_t chr);
static bool _isDiacritic(unicode_t chr);

bool LPM_LangRusEng_isCharBelongsToLang(const unicode_t * pchr)
{
    const unicode_t chr = *pchr;

    // Заглавные и строчные буквы от "А" до "я", кроме букв "Ё" и "ё"
    if(_withinBaseRussian(chr))
        return true;

    // Буква "Ё"
    if(chr == chrYoBig)
        return true;

    // Буква "ё"
    if(chr == chrYoSmall)
        return true;

    // Диакритический знак "̆ " ("кратка")
    if(chr == chrCratca)
        return true;

    // Диакритический знак "̈ " ("умлаут")
    if(chr == chrUmlaut)
        return true;

    return false;
}

bool LPM_LangRusEng_checkInputChar(unicode_t inputChr, const unicode_t * pchr)
{
    if(_isDiacritic(inputChr))
    {
        --pchr;
        if(_isDiacritic(*pchr))
            return false;
        if(inputChr == chrCratca)
            return (*pchr == chrIBig) || (*pchr == chrISmall);
        if(inputChr == chrUmlaut)
            return (*pchr == chrEBig) || (*pchr == chrESmall);
        return false;
    }
    return true;
}

const unicode_t * LPM_LangRusEng_nextChar(const unicode_t * pchr)
{
    const unicode_t chr = *pchr;

    if((chr == chrIBig) || (chr == chrISmall))
    {
        if(pchr[1] == chrCratca)
            ++pchr;
    }
    else if((chr == chrEBig) || (chr == chrESmall))
    {
        if(pchr[1] == chrUmlaut)
            ++pchr;
    }
    return ++pchr;
}

const unicode_t * LPM_LangRusEng_prevChar(const unicode_t * pchr)
{
    if((*pchr == chrCratca) || (*pchr == chrUmlaut) )
        --pchr;
    return pchr;
}

bool _withinBaseRussian(unicode_t chr)
{
    return (chr >= 0x0410) && (chr <= 0x044f);
}

bool _isDiacritic(unicode_t chr)
{
    return (chr == chrIBig) || (chr == chrISmall);
}

//const unicode_t * LPM_LangRusEng_goToChar(const unicode_t * pchr, int32_t amount)
//{
//    if(amount == 0)
//        return pchr;

//    bool goForward = amount > 0;
//    if(goForward)
//    {
//        for(size_t i = 0; i < amount; i++)
//            pchr = _gotoNextChar(pchr);
//    }
//    else
//    {
//        amount = -amount;
//        for(size_t i = 0; i < amount; i++)
//            pchr = _gotoPrevChar(pchr);
//    }
//    return pchr;
//}
