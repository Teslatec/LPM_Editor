#include "template_loader.h"
#include "lpm_editor_api.h"

uint32_t TemplateLoader_readTemplateNames(const Modules * m, LPM_File * f)
{
    (void)f;
    m->templateNameTable[0] = 1;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_readText(const Modules * m, LPM_File * f, uint8_t index)
{
    (void)f; (void)index; (void)m;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_saveText(const Modules * m, LPM_File * f, uint8_t index)
{
    (void)f; (void)index; (void)m;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_removeTemplate(const Modules * m, LPM_File * f, uint8_t index)
{
    (void)f; (void)index; (void)m;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_removeAllTemplates(const Modules * m, LPM_File * f)
{
    (void)f; (void)m;
    return LPM_EDITOR_OK;
}

bool TemplateLoader_findTemplateIndexByName
        ( const Modules * m,
          uint16_t templateName,
          uint8_t * templateIndex )
{
    (void)m, (void)templateName;
    *templateIndex = 0;
    return true;
}
