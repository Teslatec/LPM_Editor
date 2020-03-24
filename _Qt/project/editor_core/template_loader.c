#include "template_loader.h"
#include "lpm_editor_api.h"
#include <string.h>

#define SERVICE_SIZE (256)

#define EMPTY_NAME  0x0000
#define NOINIT_NAME 0xFFFF

// Переписать _fillServicePart для работы с внешней памятью

static size_t _fullSize(const LPM_EditorSystemParams * sp);
static bool _nameIsBad(const LPM_EditorSystemParams * sp, uint16_t name);
static void _fillServicePart(const LPM_Buf * buf, uint16_t name);

uint32_t TemplateLoader_readTemplateNames(const Modules * m, const LPM_EditorSystemParams * sp)
{
    size_t fullSize = _fullSize(sp);
    size_t base = fullSize;
    uint16_t name;
    LPM_Buf buf = { (uint8_t*)&name, sizeof(uint16_t) };
    buf.size = 2;
    for(size_t i = 0; i < sp->settings->maxTemplateAmount; i++)
    {
        LPM_File_read(sp->templatesFile, &buf, base - 2);
        if(LPM_File_errorOccured(sp->templatesFile))
            return LPM_EDITOR_ERROR_FLASH_READ;
        if(_nameIsBad(sp, name))
            name = EMPTY_NAME;
        m->templateNameTable[i] = name;
        base += fullSize;
        test_print("read names", i, name);
    }


    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_readText(const Modules * m, const LPM_EditorSystemParams * sp, uint8_t index)
{
    test_print("read text", index, m->templateNameTable[index]);

    size_t fullSize = _fullSize(sp);
    LPM_Buf buf = { sp->settings->textBuffer.data, sp->settings->maxTemplateSize };
    LPM_File_read(sp->templatesFile, &buf, fullSize*index);
    if(LPM_File_errorOccured(sp->templatesFile))
        return LPM_EDITOR_ERROR_FLASH_READ;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_saveText(const Modules * m, const LPM_EditorSystemParams * sp, uint8_t index)
{
    test_print("save text", index, m->templateNameTable[index]);

    size_t fullSize = _fullSize(sp);

    LPM_Buf buf = { sp->settings->textBuffer.data, fullSize };
    uint16_t name = m->templateNameTable[index];
    _fillServicePart(&buf, name);

    LPM_File_write(sp->templatesFile, &buf, fullSize*index);
    if(LPM_File_errorOccured(sp->templatesFile))
        return LPM_EDITOR_ERROR_FLASH_WRITE;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_removeTemplate(const Modules * m, const LPM_EditorSystemParams * sp, uint8_t index)
{
    (void)m, (void)sp, (void)index;
    return LPM_EDITOR_OK;
}

uint32_t TemplateLoader_removeAllTemplates(LPM_File * f)
{
    LPM_File_clear(f);
    return LPM_File_errorOccured(f) ? LPM_EDITOR_ERROR_FLASH_WRITE : LPM_EDITOR_OK;
}

uint32_t TemplateLoader_findTemplateIndexByName
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint16_t templateName,
          uint8_t * templateIndex )
{
    const uint16_t * ptr = m->templateNameTable;
    const uint16_t * const end = ptr + sp->settings->templatebadNameAmount;
    uint8_t index = 0;
    for( ; ptr != end; ptr++, index++)
    {
        if(*ptr == templateName)
        {
            *templateIndex = index;
            return LPM_EDITOR_OK;
        }
    }
    return LPM_EDITOR_ERROR_BAD_TEMPLATE_NAME;
}

size_t _fullSize(const LPM_EditorSystemParams * sp)
{
    return sp->settings->maxTemplateSize + SERVICE_SIZE;
}

bool _nameIsBad(const LPM_EditorSystemParams * sp, uint16_t name)
{
    const uint16_t * ptr = sp->settings->templateBadNameTable;
    const uint16_t * const end = ptr + sp->settings->templatebadNameAmount;
    for( ; ptr != end; ptr++)
        if(*ptr == name)
            return true;
    return false;
}

void _fillServicePart(const LPM_Buf * buf, uint16_t name)
{
    memset(buf->data + buf->size - SERVICE_SIZE, 0, SERVICE_SIZE - 2);
    buf->data[buf->size - 2] = (uint8_t)name;
    buf->data[buf->size - 1] = (uint8_t)(name >> 8);
}
