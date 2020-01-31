#ifndef LPM_ERROR_H
#define LPM_ERROR_H

#include <stdint.h>
#include <stdbool.h>

typedef int32_t error_t;

inline bool LPM_ErrorOccured(error_t err) { return err != 0; }

/*
 * Тут определить коды ошибок
 */

#define LPM_NO_ERROR 0

#endif // LPM_ERROR_H
