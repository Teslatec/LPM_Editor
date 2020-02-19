#ifndef CORE_H
#define CORE_H

#include "modules.h"

typedef struct Core
{
    Modules * modules;
    LPM_SelectionCursor textCursor;
} Core;

void Core_init(Core * o, Modules * modules);

void Core_exec(Core * o);

#endif // CORE_H
