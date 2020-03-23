#include "lang_rus_eng.h"

static const unicode_t chrYoBig   = 0x0401;
static const unicode_t chrYoSmall = 0x0451;
static const unicode_t chrCratca  = 0x0306;
static const unicode_t chrUmlaut  = 0x0308;
static const unicode_t chrIBig    = 0x0418;
static const unicode_t chrISmall  = 0x0438;
static const unicode_t chrEBig    = 0x0415;
static const unicode_t chrESmall  = 0x0435;
static const unicode_t chrRussianBegin = 0x0410;
static const unicode_t chrRussianEnd   = 0x044f;

static bool _belongsBaseRussian(unicode_t chr);
static bool _isChrLetterI(unicode_t chr);
static bool _isChrLetterE(unicode_t chr);
static bool _isChrCratca(unicode_t chr);
static bool _isChrUmlaut(unicode_t chr);

bool Lang_RusEng_checkInputChar(unicode_t inputChr, const unicode_t * pchr, bool * isDiacritic)
{
    if(_belongsBaseRussian(inputChr))
        return true;

    --pchr;

    if(_isChrCratca(inputChr))
    {
        *isDiacritic = true;
        return _isChrLetterI(*pchr);
    }

    if(_isChrUmlaut(inputChr))
    {
        *isDiacritic = true;
        return _isChrLetterE(*pchr);
    }

    return true;
}

const unicode_t * Lang_RusEng_nextChar(const unicode_t * pchr)
{
    if(_isChrLetterI(pchr[0]))
    {
        if(_isChrCratca(pchr[1]))
            ++pchr;
    }
    else if(_isChrLetterE(pchr[0]))
    {
        if(_isChrUmlaut(pchr[1]))
            ++pchr;
    }
    return ++pchr;
}

const unicode_t * Lang_RusEng_prevChar(const unicode_t * pchr)
{
    if(_isChrCratca(*pchr) || _isChrUmlaut(*pchr))
        --pchr;
    return pchr;
}

bool _belongsBaseRussian(unicode_t chr)
{
    // Заглавные и строчные буквы от "А" до "я"
    if((chr >= chrRussianBegin) && (chr <= chrRussianEnd))
        return true;

    // Буквы "Ё" и "ё" - отдельно
    if((chr == chrYoBig) || (chr == chrYoSmall))
        return true;

    return false;
}

bool _isChrLetterI(unicode_t chr) { return (chr == chrIBig) || (chr == chrISmall); }
bool _isChrLetterE(unicode_t chr) { return (chr == chrEBig) || (chr == chrESmall); }
bool _isChrCratca(unicode_t chr)  { return chr == chrCratca; }
bool _isChrUmlaut(unicode_t chr)  { return chr == chrUmlaut; }
