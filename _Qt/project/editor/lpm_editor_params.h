#ifndef LPM_EDITOR_PARAMS_H
#define LPM_EDITOR_PARAMS_H

#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"
#include "lpm_file.h"

typedef enum LPM_EndOfLineType
{
    LPM_END_OF_LINE_TYPE_CR,
    LPM_END_OF_LINE_TYPE_LF,
    LPM_END_OF_LINE_TYPE_CRLF
} LPM_EndOfLineType;

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

#endif // LPM_EDITOR_PARAMS_H
