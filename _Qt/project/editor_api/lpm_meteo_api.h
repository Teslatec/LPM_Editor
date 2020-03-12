#ifndef LPM_METEO_API_H
#define LPM_METEO_API_H

#include "lpm_structs.h"

typedef enum LPM_Meteo
{
    LPM_METEO_GSM_CURR_CIRC_1,
    LPM_METEO_GSM_CURR_CIRC_2,
    LPM_METEO_GSM_CURR_ADDR_1,
    LPM_METEO_GSM_CURR_ADDR_2,
    LPM_METEO_GSM_IMPR_CIRC,
    LPM_METEO_GSM_IMPR_ADDR,
    LPM_METEO_VMO_CURR_CIRC,
    LPM_METEO_VMO_CURR_ADDR,
    LPM_METEO_VMO_NEW_ADDR,
    LPM_METEO_FAX_CHAIN_CURR,
} LPM_Meteo;

typedef struct LPM_MeteoFxns
{
    bool (*checkFormat)(const LPM_Buf * msgBuffer, LPM_Meteo format);
    void (*encodeTo)(LPM_Buf * msgBuffer, LPM_Meteo format);
    void (*decodeFrom)(LPM_Buf * msgBuffer, LPM_Meteo format);
} LPM_MeteoFxns;

static inline bool LPM_Meteo_checkFormat
        ( const LPM_MeteoFxns * fxns,
          const LPM_Buf * msgBuffer,
          LPM_Meteo format )
{
    return (*fxns->checkFormat)(msgBuffer, format);
}

static inline void LPM_Meteo_encodeTo
        ( const LPM_MeteoFxns * fxns,
          LPM_Buf * msgBuffer,
          LPM_Meteo format )
{
    (*fxns->encodeTo)(msgBuffer, format);
}

static inline void LPM_Meteo_decodeFrom
        ( const LPM_MeteoFxns * fxns,
          LPM_Buf * msgBuffer,
          LPM_Meteo format )
{
    (*fxns->decodeFrom)(msgBuffer, format);
}


#endif // LPM_METEO_API_H
