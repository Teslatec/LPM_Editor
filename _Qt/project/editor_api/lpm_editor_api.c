#include "lpm_editor_api.h"
#include "controller.h"

uint32_t LPM_API_execEditor
    ( const LPM_EditorUserParams * userParams,
      const LPM_EditorSystemParams * systemParams )
{
    return Controller_exec(userParams, systemParams);
}


size_t LPM_API_getDesiredHeapSize
        (const LPM_EditorSystemParams * systemParams)
{
    return Controller_calcDesiredHeapSize(systemParams);
}
