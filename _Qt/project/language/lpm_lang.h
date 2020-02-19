#ifndef LPM_LANG_H
#define LPM_LANG_H

#include "lpm_unicode.h"

typedef enum LPM_LangSet
{
    LPM_LANG_RUS_ENG,
    LPM_LANG_LAO_ENG
} LPM_LangSet;

typedef struct LPM_Lang
{
    bool (*isCharBelongsToLang)(const unicode_t *);
    bool (*checkInputChar)(unicode_t, const unicode_t *);
    const unicode_t * (*nextChar)(const unicode_t *);
    const unicode_t * (*prevChar)(const unicode_t *);
} LPM_Lang;

bool LPM_Lang_init(LPM_Lang * lang, LPM_LangSet langSet);

inline bool LPM_Lang_isCharBelongsToLang
        ( const LPM_Lang * lang,
          const unicode_t * pchr )
{
    return (*lang->isCharBelongsToLang)(pchr);
}

inline bool LPM_Lang_checkInputChar
        ( const LPM_Lang * lang,
          unicode_t inputChr,
          const unicode_t * pchr )
{
    return (*lang->checkInputChar)(inputChr, pchr);
}

inline const unicode_t * LPM_Lang_nextChar
        ( const LPM_Lang * lang,
          const unicode_t * pchr )
{
    return (*lang->nextChar)(pchr);
}

inline const unicode_t * LPM_Lang_prevChar
        ( const LPM_Lang * lang,
          const unicode_t * pchr )
{
    return (*lang->prevChar)(pchr);
}


#endif // LPM_LANG_H
