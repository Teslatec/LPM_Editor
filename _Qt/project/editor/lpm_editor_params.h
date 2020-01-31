#ifndef LPM_EDITOR_PARAMS_H
#define LPM_EDITOR_PARAMS_H

#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"
#include "lpm_file.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LPM_EditorParams
{
    LPM_UnicodeKeyboard * kbd;
    LPM_UnicodeDisplay  * dsp;
    LPM_Buf * textBuffer;
} LPM_EditorParams;

typedef struct LPM_MeteoEditorParams
{} LPM_MeteoEditorParams;

typedef struct LPM_TemplateEditorParams
{} LPM_TemplateEditorParams;

typedef struct LPM_InsertionEditorParams
{} LPM_InsertionEditorParams;

#ifdef __cplusplus
}
#endif

#endif // LPM_EDITOR_PARAMS_H
