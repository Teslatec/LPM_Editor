#ifndef LPM_LANG_API_H
#define LPM_LANG_API_H

#include "lpm_unicode.h"

typedef enum LPM_Lang
{
    LPM_LANG_RUS_ENG,
    LPM_LANG_LAO_ENG
} LPM_Lang;

typedef struct LPM_LangFxns
{
    bool (*checkInputChar)(unicode_t, const unicode_t *);
    const unicode_t * (*nextChar)(const unicode_t *);
    const unicode_t * (*prevChar)(const unicode_t *);
} LPM_LangFxns;


static inline bool LPM_Lang_checkInputChar
        ( const LPM_LangFxns * o,
          unicode_t inputChr,
          const unicode_t * pchr )
{
    return (*o->checkInputChar)(inputChr, pchr);
}

static inline const unicode_t * LPM_Lang_nextChar
        ( const LPM_LangFxns * o,
          const unicode_t * pchr )
{
    return (*o->nextChar)(pchr);
}

static inline const unicode_t * LPM_Lang_prevChar
        ( const LPM_LangFxns * o,
          const unicode_t * pchr )
{
    return (*o->prevChar)(pchr);
}


#endif // LPM_LANG_API_H
