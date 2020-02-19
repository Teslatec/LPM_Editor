#ifndef LPM_EDITOR_API_H
#define LPM_EDITOR_API_H

#include "lpm_editor_params.h"

void LPM_launchEditor(const LPM_EditorParams * params);
void LPM_launchMeteoEditor(const LPM_EditorParams * params);
void LPM_launchTemplateEditor(const LPM_TemplateEditorParams * params);
void LPM_launchInsertionEditor(const LPM_InsertionEditorParams * params);

#endif // LPM_EDITOR_API_H
