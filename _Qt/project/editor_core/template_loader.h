#ifndef TEMPLATE_LOADER_H
#define TEMPLATE_LOADER_H

#include "modules.h"
#include "lpm_file.h"
#include "lpm_editor_api.h"

uint32_t TemplateLoader_readTemplateNames(const Modules * m, const LPM_EditorSystemParams * sp);
uint32_t TemplateLoader_readText(const Modules * m, const LPM_EditorSystemParams * sp, uint8_t index);
uint32_t TemplateLoader_saveText(const Modules * m, const LPM_EditorSystemParams * sp, uint8_t index);

uint32_t TemplateLoader_findTemplateIndexByName
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint16_t templateName,
          uint8_t * templateIndex );

uint32_t TemplateLoader_removeTemplate(const Modules * m, const LPM_EditorSystemParams * sp, uint8_t index);
uint32_t TemplateLoader_removeAllTemplates(LPM_File * f);

#endif // TEMPLATE_LOADER_H
