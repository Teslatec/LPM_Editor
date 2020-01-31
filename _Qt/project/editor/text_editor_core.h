#ifndef TEXT_EDITOR_CORE_H
#define TEXT_EDITOR_CORE_H

#include "text_editor_page_map.h"

typedef struct TextEditorCore
{
    TextEditorPageMap pageMap;
} TextEditorCore;

void TextEditorCore_init( TextEditorCore * obj,
                          const TextEditorLineTable * lineTable );
void TextEditorCore_exec(TextEditorCore * obj);

#endif // TEXT_EDITOR_CORE_H
