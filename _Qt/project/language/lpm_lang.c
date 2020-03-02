#include "lpm_lang.h"
#include "lpm_lang_rus_eng.h"

bool LPM_Lang_init(LPM_Lang * lang, LPM_LangSet langSet)
{
    if(langSet == LPM_LANG_RUS_ENG)
    {
        lang->checkInputChar = &LPM_LangRusEng_checkInputChar;
        lang->nextChar       = &LPM_LangRusEng_nextChar;
        lang->prevChar       = &LPM_LangRusEng_prevChar;
        return true;
    }
    return false;
}
