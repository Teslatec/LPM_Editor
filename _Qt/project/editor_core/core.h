#ifndef CORE_H
#define CORE_H

#include "modules.h"
#include "lpm_editor_api.h"
#include "lpm_unicode_keyboard.h"

typedef struct Core
{
    Modules * modules;
    LPM_UnicodeDisplay * display;
    LPM_SelectionCursor textCursor;
    LPM_SelectionCursor undoTextCursor;
    LPM_EndOfLineType endOfLine;
    bool hasActionToUndo;
} Core;

void Core_init
        ( Core * o,
          Modules * modules,
          LPM_UnicodeDisplay * display,
          LPM_EndOfLineType endOfLine );

void Core_exec(Core * o);

#endif // CORE_H
