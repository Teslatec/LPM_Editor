#ifndef LPM_LANG_RUS_ENG_H
#define LPM_LANG_RUS_ENG_H

#include "lpm_unicode.h"

bool LPM_LangRusEng_checkInputChar(unicode_t inputChr, const unicode_t * pchr);
const unicode_t * LPM_LangRusEng_nextChar(const unicode_t * pchr);
const unicode_t * LPM_LangRusEng_prevChar(const unicode_t * pchr);

#endif // LPM_LANG_RUS_ENG_H
