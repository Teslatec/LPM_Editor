#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "lpm_editor_api.h"

typedef struct Controller
{} Controller;

uint32_t Controller_exec
        ( const LPM_EditorUserParams * userParams,
          const LPM_EditorSystemParams * systemParams );

size_t Controller_calcDesiredHeapSize(const LPM_EditorSystemParams * p);

#endif // CONTROLLER_H
