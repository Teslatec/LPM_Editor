#ifndef TEXT_EDITOR_CORE_H
#define TEXT_EDITOR_CORE_H

#include "text_editor_modules.h"
#include "text_editor_page_map.h"

typedef struct TextEditorCore
{
    TextEditorPageMap pageMap;
    TextEditorModules * modules;
} TextEditorCore;

void TextEditorCore_init(TextEditorCore * o, TextEditorModules * modules);

void TextEditorCore_exec(TextEditorCore * o);

#endif // TEXT_EDITOR_CORE_H
