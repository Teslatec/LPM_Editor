#ifndef PAGE_FORMATTER_H
#define PAGE_FORMATTER_H

#include "lpm_structs.h"
#include "lpm_unicode.h"
#include "modules.h"

#define PAGE_LINE_AMOUNT 16
#define PAGE_CHAR_AMOUNT 64
#define PAGE_GROUP_AMOUNT 4
#define PAGES_IN_GROUP  4

typedef struct LineMap
{
    uint16_t crc;
    uint8_t fullLen;
    uint8_t payloadLen;
    uint8_t restLen;
    bool endsWithEndl;
} LineMap;

typedef struct PageNavigation
{
    size_t groupBaseTable[PAGE_GROUP_AMOUNT];
    size_t currGroupIndex;
    size_t currPageIndex;
} PageNavigation;

typedef struct PageStruct
{
    LineMap prevLastLine;
    LineMap lineMapTable[PAGE_LINE_AMOUNT];
    size_t base;
    bool lastPageReached;
} PageStruct;


typedef struct PageFormatter
{
    const Modules * modules;
    PageNavigation pageNavi;
    PageStruct pageStruct;
    uint32_t lineChangedFlags;
    LPM_DisplayCursor displayCursor;
    LPM_SelectionCursor textCursor;
    bool selectBackward;
} PageFormatter;

void PageFormatter_init
        ( PageFormatter * o,
          const Modules * modules );

void PageFormatter_startWithPageAtTextPosition
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs );

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * curs );

void PageFormatter_updatePageWhenCursorMoved
        ( PageFormatter * o,
          uint32_t moveFlags ,
          LPM_SelectionCursor * textCurs );

void PageFormatter_updateWholePage(PageFormatter * o);

void PageFormatter_updateDisplay(PageFormatter * o);

size_t PageFormatter_getCurrLinePos(PageFormatter * o);
size_t PageFormatter_getCurrLineLen(PageFormatter * o);

size_t PageFormatter_getCurrPagePos(PageFormatter * o);
size_t PageFormatter_getCurrPageLen(PageFormatter * o);

#endif // PAGE_FORMATTER_H
