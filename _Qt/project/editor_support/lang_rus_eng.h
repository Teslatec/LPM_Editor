#ifndef LANG_RUS_ENG_H
#define LANG_RUS_ENG_H

#include "lpm_unicode.h"

bool Lang_RusEng_checkInputChar(unicode_t inputChr, const unicode_t * pchr);
const unicode_t * Lang_RusEng_nextChar(const unicode_t * pchr);
const unicode_t * Lang_RusEng_prevChar(const unicode_t * pchr);

#endif // LANG_RUS_ENG_H
