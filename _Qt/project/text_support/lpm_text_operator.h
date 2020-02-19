#ifndef TEXT_OPERATOR_H
#define TEXT_OPERATOR_H

#include "lpm_unicode.h"

struct LPM_Lang;

typedef struct LPM_TextOperator
{
    struct LPM_Lang * lang;
} LPM_TextOperator;

inline void LPM_TextOperator_init
    (LPM_TextOperator * o, struct LPM_Lang * lang) { o->lang = lang; }

bool LPM_TextOperator_checkInputChar
    ( LPM_TextOperator * o,
      unicode_t inputChr,
      const unicode_t * pchr );

const unicode_t * LPM_TextOperator_nextChar
    ( LPM_TextOperator * o,
      const unicode_t * pchr );

const unicode_t * LPM_TextOperator_prevChar
    ( LPM_TextOperator * o,
      const unicode_t * pchr );

bool LPM_TextOperator_atEndOfText(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_atEndOfLine(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_atSpace(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_atTab(LPM_TextOperator * o, const unicode_t * pchr);

bool LPM_TextOperator_checkText(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_correctText(LPM_TextOperator * o, unicode_t * pchr);

#endif // TEXT_OPERATOR_H
