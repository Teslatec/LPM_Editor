#ifndef TEXT_EDITOR_CORE_H
#define TEXT_EDITOR_CORE_H

#include "text_editor_modules.h"

typedef struct TextEditorCore
{
    TextEditorModules * modules;
    LPM_SelectionCursor textCursor;
} TextEditorCore;

void TextEditorCore_init(TextEditorCore * o, TextEditorModules * modules);

void TextEditorCore_exec(TextEditorCore * o);

#endif // TEXT_EDITOR_CORE_H
