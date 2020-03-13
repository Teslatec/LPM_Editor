#ifndef CORE_H
#define CORE_H

#include "modules.h"
#include "lpm_editor_api.h"
#include "lpm_unicode_keyboard.h"

typedef struct Core
{
    const Modules * modules;
    LPM_UnicodeDisplay * display;
    LPM_SelectionCursor textCursor;
    LPM_SelectionCursor undoTextCursor;
    unicode_t insertionBorderChar;
    LPM_EndOfLineType endOfLine;
    uint8_t tabSpaceAmount;
    uint8_t flags;
} Core;

void Core_init
        ( Core * o,
          const Modules * modules,
          const LPM_EditorUserParams * userParams,
          const LPM_EditorSystemParams * systemParams );

uint32_t Core_exec(Core * o);

void Core_setReadOnly(Core * o);
void Core_setTemplateMode(Core * o);
void Core_setInsertionsMode(Core * o);

#endif // CORE_H
