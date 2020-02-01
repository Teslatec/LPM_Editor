#ifndef TEXT_EDITOR_PAGE_MAP_H
#define TEXT_EDITOR_PAGE_MAP_H

#include "lpm_structs.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct TextEditorLineMap
{
    uint32_t begin;
    uint32_t end;
} TextEditorLineMap;

typedef struct TextEditorLineTable
{
    TextEditorLineMap * buf;
    uint8_t size;
} TextEditorLineTable;

typedef struct TextEditorPageMap
{
    TextEditorLineTable lineTable;
    LPM_DisplayCursor   displayCursor;
    LPM_SelectionCursor selectionCursor;
    uint16_t currentPage;
    uint16_t pageAmount;
    bool isReplacementMode;
    bool isTextHighlighted;
} TextEditorPageMap;

#endif // TEXT_EDITOR_PAGE_MAP_H
