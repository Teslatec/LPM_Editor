#ifndef LPM_STRUCTS_H
#define LPM_STRUCTS_H

#include <stdint.h>

typedef struct LPM_Buf
{
    uint8_t * data;
    size_t size;
} LPM_Buf;

typedef struct LPM_Point
{
    uint8_t x;
    uint8_t y;
} LPM_Point;

/*
 * Курсор дисплея:
 *  begin - точка, в которой начинается выделение текста
 *  end   - точка, в которой заканчивается выделение текста
 * Если begin == end, то выделения текста нет, вывести курсор на это место
 * Гарантируется, что при отсчете от левого верхнего угла точка begin
 *  всегда "первее" точки end
 */
typedef struct LPM_Cursor
{
    LPM_Point begin;
    LPM_Point end;
} LPM_Cursor;


#endif // LPM_STRUCTS_H
