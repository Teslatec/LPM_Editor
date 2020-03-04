#ifndef CORE_H
#define CORE_H

#include "modules.h"
#include "lpm_editor_params.h"

typedef struct Core
{
    Modules * modules;
    LPM_SelectionCursor textCursor;
    LPM_SelectionCursor undoTextCursor;
    LPM_EndOfLineType endOfLineType;
    bool hasActionToUndo;
} Core;

void Core_init(Core * o, Modules * modules, LPM_EndOfLineType endOfLineType);

void Core_exec(Core * o);

#endif // CORE_H
