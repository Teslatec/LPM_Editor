#include "lpm_editor_api.h"
#include "text_editor_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

void LPM_launchEditor(const LPM_EditorParams * params)
{
    TextEditorControler_exec(params);
}

void LPM_launchMeteoEditor(const LPM_EditorParams * params)
{
    (void)params;
}

void LPM_launchTemplateEditor(const LPM_TemplateEditorParams * params)
{
    (void)params;
}

void LPM_launchInsertionEditor(const LPM_InsertionEditorParams * params)
{
    (void)params;
}

#ifdef __cplusplus
}
#endif
