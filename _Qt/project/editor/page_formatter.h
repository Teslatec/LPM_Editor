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
    uint8_t fullLen;
    uint8_t payloadLen;
    uint8_t restLen;
} LineMap;

typedef struct PageFormatter
{
    const Modules * modules;
    size_t groupBaseTable[PAGE_GROUP_AMOUNT];
    size_t currPageBase;
    size_t currGroupIndex;
    size_t currPageIndex;
    LineMap lineMap[PAGE_LINE_AMOUNT];
    LineMap prevPageLastLineMap;
    uint32_t lineChangedFlags;
    bool lastPageReached;
    LPM_DisplayCursor signPlaceCursor;
    LPM_DisplayCursor displayCursor;
} PageFormatter;

void PageFormatter_init
        ( PageFormatter * o,
          const Modules * modules );

void PageFormatter_startWithPageAtTextPosition
        (PageFormatter * o,
          LPM_SelectionCursor * curs );

void PageFormatter_updatePageByTextChanging
        ( PageFormatter * o,
          const LPM_SelectionCursor * changedTextArea,
          const LPM_SelectionCursor * newTextCursor );

void PageFormatter_updatePageByDisplayCursorChanging
        ( PageFormatter * o,
          LPM_SelectionCursor * textArea,
          uint32_t flags );

void PageFormatter_updateDisplay(PageFormatter * o);

#endif // PAGE_FORMATTER_H
