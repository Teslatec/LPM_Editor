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
    LPM_EndlType endlType;
    uint8_t tabSpaceAmount;
    uint8_t flags;
} Core;

void Core_init
        ( Core * o,
          const Modules * modules,
          const LPM_EditorSystemParams * systemParams );

uint32_t Core_exec(Core * o);

void Core_setReadOnly(Core * o);
void Core_setTemplateMode(Core * o);
void Core_setInsertionsMode(Core * o);
void Core_checkTemplateFormat(Core * o, uint32_t * badPageMap);
bool Core_checkInsertionFormatAndReadNameIfOk(Core * o, uint16_t * templateName);

#endif // CORE_H
