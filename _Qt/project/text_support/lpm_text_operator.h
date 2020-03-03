#ifndef TEXT_OPERATOR_H
#define TEXT_OPERATOR_H

#include "lpm_unicode.h"

struct LPM_Lang;

typedef struct LPM_TextOperator
{
    struct LPM_Lang * lang;
    Unicode_Buf specChars;
} LPM_TextOperator;

typedef struct LPM_TextLineMap
{
    const unicode_t * nextLine;
    const unicode_t * printBorder;
    size_t lenInChr;
    bool endsWithEndl;
} LPM_TextLineMap;

void LPM_TextOperator_init
    ( LPM_TextOperator * o,
      struct LPM_Lang * lang,
      const Unicode_Buf * specChars );

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

bool LPM_TextOperator_analizeLine(
          LPM_TextOperator * o,
          const unicode_t  * pchr,
          size_t maxLenInChrs,
          LPM_TextLineMap * lineMap );

const unicode_t * LPM_TextOperator_nextNChar
    ( LPM_TextOperator * o,
      const unicode_t * pchr,
      size_t chrAmount );

size_t LPM_TextOperator_calcChrAmount
    ( LPM_TextOperator * o,
      const unicode_t * begin,
      const unicode_t * end );

bool LPM_TextOperator_atEndOfText(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_atEndOfLine(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_atSpace(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_atTab(LPM_TextOperator * o, const unicode_t * pchr);

bool LPM_TextOperator_checkText(LPM_TextOperator * o, const unicode_t * pchr);
bool LPM_TextOperator_correctText(LPM_TextOperator * o, unicode_t * pchr);

#endif // TEXT_OPERATOR_H
