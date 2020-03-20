#ifndef PAGE_FORMATTER_H
#define PAGE_FORMATTER_H

#include "lpm_structs.h"
#include "lpm_unicode.h"
#include "lpm_editor_api.h"
#include "modules.h"

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
    size_t * groupBaseTable;
    size_t currGroupIndex;
    size_t currPageIndex;
} PageNavigation;

typedef struct PageStruct
{
    LineMap prevLastLine;
    LineMap * lineMapTable;
    size_t base;
    bool lastPageReached;
} PageStruct;


typedef struct PageFormatter
{
    const Modules * modules;
    const LPM_EditorPageParams * pageParams;
    LPM_UnicodeDisplay * display;
    PageNavigation pageNavi;
    PageStruct pageStruct;
    uint32_t lineChangedFlags;
    LPM_DisplayCursor displayCursor;
    LPM_SelectionCursor textCursor;
    uint8_t spaceAmount;
    bool selectBackward;
} PageFormatter;

void PageFormatter_init
        ( PageFormatter * o,
          const Modules * modules,
          LPM_UnicodeDisplay * display,
          const LPM_EditorPageParams * pageParams );

void PageFormatter_startWithPageAtTextPosition
        ( PageFormatter * o,
          const LPM_SelectionCursor * txtCurs );

void PageFormatter_updatePageWhenTextChanged
        ( PageFormatter * o,
          const LPM_SelectionCursor * txtCurs );

void PageFormatter_updatePageWhenCursorMoved
        ( PageFormatter * o,
          uint32_t moveFlags ,
          LPM_SelectionCursor * txtCurs );

void PageFormatter_updateWholePage(PageFormatter * o);

void PageFormatter_updateDisplay(PageFormatter * o);

size_t PageFormatter_getCurrLinePos(PageFormatter * o);
size_t PageFormatter_getCurrLineLen(PageFormatter * o);

size_t PageFormatter_getCurrPagePos(PageFormatter * o);
size_t PageFormatter_getCurrPageLen(PageFormatter * o);

#endif // PAGE_FORMATTER_H
