#ifndef TEXT_OPERATOR_H
#define TEXT_OPERATOR_H

#include "lpm_unicode.h"

struct LPM_LangFxns;

typedef struct TextOperator
{
    struct LPM_LangFxns * lang;
    Unicode_Buf specChars;
} TextOperator;

typedef struct LPM_TextLineMap
{
    const unicode_t * nextLine;
    const unicode_t * printBorder;
    size_t lenInChr;
    bool endsWithEndl;
} LPM_TextLineMap;

void TextOperator_init
    ( TextOperator * o,
      struct LPM_LangFxns * lang,
      const Unicode_Buf * specChars );

bool TextOperator_checkInputChar
    ( TextOperator * o,
      unicode_t inputChr,
      const unicode_t * pchr );

const unicode_t * TextOperator_nextChar
    ( TextOperator * o,
      const unicode_t * pchr );

const unicode_t * TextOperator_prevChar
    ( TextOperator * o,
      const unicode_t * pchr );

bool TextOperator_analizeLine(
          TextOperator * o,
          const unicode_t  * pchr,
          size_t maxLenInChrs,
          LPM_TextLineMap * lineMap );

const unicode_t * TextOperator_nextNChar
    ( TextOperator * o,
      const unicode_t * pchr,
      size_t chrAmount );

size_t TextOperator_calcChrAmount
    ( TextOperator * o,
      const unicode_t * begin,
      const unicode_t * end );

bool TextOperator_atEndOfText(TextOperator * o, const unicode_t * pchr);
bool TextOperator_atEndOfLine(TextOperator * o, const unicode_t * pchr);
bool TextOperator_atSpace(TextOperator * o, const unicode_t * pchr);
bool TextOperator_atTab(TextOperator * o, const unicode_t * pchr);

bool TextOperator_checkText(TextOperator * o, const unicode_t * pchr);
bool TextOperator_correctText(TextOperator * o, unicode_t * pchr);

#endif // TEXT_OPERATOR_H
