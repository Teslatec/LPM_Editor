#ifndef TEXT_EDITOR_PAGE_MAP_H
#define TEXT_EDITOR_PAGE_MAP_H

#include "lpm_structs.h"
#include <stdint.h>
#include <stdbool.h>

#define TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT 16
#define TEXT_EDITOR_PAGE_MAP_CHAR_AMOUNT 64

typedef struct TextEditorLineTableItem
{
    uint32_t begin;
    uint32_t end;
} TextEditorLineTableItem;

typedef struct TextEditorPageMap
{
    TextEditorLineTableItem lineTable[TEXT_EDITOR_PAGE_MAP_CHAR_AMOUNT];
    LPM_DisplayCursor displayCursor;
    LPM_SelectionCursor selectionCursor;
    uint16_t changedLineFlags;
    uint16_t currentPage;
    uint16_t pageAmount;
    bool isReplacementMode;
    bool isTextHighlighted;
} TextEditorPageMap;

#if (TEXT_EDITOR_PAGE_MAP_LINE_AMOUNT > 16)
#error "Please redefine "changedLineFlags" field type in TextEditorPageMap \
structure to support more than 16 lines (to uint32_t for example) "
#endif

#endif // TEXT_EDITOR_PAGE_MAP_H
