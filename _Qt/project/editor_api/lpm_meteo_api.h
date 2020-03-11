#ifndef LPM_METEO_API_H
#define LPM_METEO_API_H


// -----------------------------------------------------------------------------
//  Поддержка форматов метеосообщений

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


// Аналогично Lang

#endif // LPM_METEO_API_H
