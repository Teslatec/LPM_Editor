#ifndef LPM_ENCODING_API_H
#define LPM_ENCODING_API_H

#include "lpm_structs.h"

typedef enum LPM_Encoding
{
    LPM_ENCODING_UNICODE_UCS2LE,
    LPM_ENCODING_ASCII,
    LPM_ENCODING_KOI_7H0,
    LPM_ENCODING_KOI_7H1,
    LPM_ENCODING_KOI_8
} LPM_Encoding;

typedef struct LPM_EncodingFxns
{
    // maxSize - масимальный размер текста в байтах ДЛЯ КОДИРОВКИ UNICODE!!!
    bool (*checkText)(const LPM_Buf * text, LPM_Encoding encoding, size_t maxSize);
    void (*encodeTo)(LPM_Buf * text, LPM_Encoding encoding);
    void (*decodeFrom)(LPM_Buf * text, LPM_Encoding encoding);
} LPM_EncodingFxns;

static inline bool LPM_Encoding_checkText
        ( const LPM_EncodingFxns * fxns,
          const LPM_Buf * text,
          LPM_Encoding encoding,
          size_t maxSize )
{
    return (*fxns->checkText)(text, encoding, maxSize);
}

static inline void LPM_Encoding_encodeTo
        ( const LPM_EncodingFxns * fxns,
          LPM_Buf * text,
          LPM_Encoding encoding )
{
    (*fxns->encodeTo)(text, encoding);
}

static inline void LPM_Encoding_decodeFrom
        ( const LPM_EncodingFxns * fxns,
          LPM_Buf * text,
          LPM_Encoding encoding )
{
    (*fxns->decodeFrom)(text, encoding);
}


#endif // LPM_ENCODING_API_H