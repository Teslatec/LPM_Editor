#include "lpm_editor_api.h"
#include "controller.h"

uint32_t LPM_API_execEditor(const LPM_EditorParams * params)
{
    Controller_exec(params);
    return 0;
}
