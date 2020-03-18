#ifndef TEMPLATE_LOADER_H
#define TEMPLATE_LOADER_H

#include "modules.h"
#include "lpm_file.h"

uint32_t TemplateLoader_readTemplateNames(const Modules * m, LPM_File * f);
uint32_t TemplateLoader_readText(const Modules * m, LPM_File * f, uint8_t index);
uint32_t TemplateLoader_saveText(const Modules * m, LPM_File * f, uint8_t index);
uint32_t TemplateLoader_removeTemplate(const Modules * m, LPM_File * f, uint8_t index);
uint32_t TemplateLoader_removeAllTemplates(const Modules * m, LPM_File * f);

bool TemplateLoader_findTemplateIndexByName
        ( const Modules * m,
          uint16_t templateName,
          uint8_t * templateIndex );

#endif // TEMPLATE_LOADER_H
