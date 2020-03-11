#ifndef LPM_EDITOR_API_H
#define LPM_EDITOR_API_H

#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"
#include "lpm_file.h"
#include "lpm_lang_api.h"
#include "lpm_encoding_api.h"
#include "lpm_meteo_api.h"

typedef enum LPM_EndOfLineType
{
    LPM_END_OF_LINE_TYPE_CR,
    LPM_END_OF_LINE_TYPE_LF,
    LPM_END_OF_LINE_TYPE_CRLF,
    LPM_END_OF_LINE_TYPE_AUTO
} LPM_EndOfLineType;

typedef enum LPM_CursorInitPos
{
    LPM_CURSOR_INIT_POS_BEGIN,
    LPM_CURSOR_INIT_POS_END,
} LPM_CursorInitPos;

typedef enum LPM_EditorMode
{
    LPM_EDITOR_MODE_TEXT_NEW,
    LPM_EDITOR_MODE_TEXT_EDIT,
    LPM_EDITOR_MODE_TEXT_VIEW,
    LPM_EDITOR_MODE_METEO_NEW,
    LPM_EDITOR_MODE_METEO_EDIT,
    LPM_EDITOR_MODE_FAX_CHAIN,
    LPM_EDITOR_MODE_TEMPLATE_NEW,
    LPM_EDITOR_MODE_TEMPLATE_EDIT,
    LPM_EDITOR_MODE_TEMP_INS_NEW,
    LPM_EDITOR_MODE_TEMP_INS_EDIT,
    LPM_EDITOR_MODE_TEMP_INS_VIEW
} LPM_EditorMode;

typedef enum LPM_EditorError
{
    LPM_EDITOR_ERROR_TEXT_SIZE           = (1u << 31),
    LPM_EDITOR_ERROR_NO_TEXT             = (1u << 30),
    LPM_EDITOR_ERROR_NO_PLACE_TO_PRINT   = (1u << 29),
    LPM_EDITOR_ERROR_DISPLAY             = (1u << 28),
    LPM_EDITOR_ERROR_KEYBOARD            = (1u << 27),
    LPM_EDITOR_ERROR_FLASH_READ          = (1u << 26),
    LPM_EDITOR_ERROR_FLASH_WRITE         = (1u << 25),
    LPM_EDITOR_ERROR_BAD_METEO_FORMAT    = (1u << 24),
    LPM_EDITOR_ERROR_BAD_TEMPLATE_FORMAT = (1u << 23),
    LPM_EDITOR_ERROR_BAD_TEMPLATE_NAME   = (1u << 22),
    LPM_EDITOR_ERROR_BAD_HEAP_SIZE       = (1u << 21),
} LPM_EditorError;

typedef enum LPM_EditorWarning
{
    LPM_EDITOR_WARNING_DIFF_ENDLS   = (1u <<  1),
    LPM_EDITOR_WARNING_BAD_CHAR_SEQ = (1u <<  0),
} LPM_EditorWarning;

typedef struct LPM_EditorParams
{
    LPM_EditorMode mode;
    LPM_UnicodeDisplay * display;
    LPM_UnicodeKeyboard * keyboard;
    LPM_File * paramFile;
    size_t paramOffset;
    LPM_File * guiTextsFile;
    LPM_File * supportFxnsMapFile;
    LPM_File * templatesFile;
    LPM_EndOfLineType endOfLineType;
    LPM_CursorInitPos cursorInitPos;
    LPM_Encoding encodingFrom;
    LPM_Encoding encodingTo;
    bool prepareToPrint;
    uint8_t lineBeginSpaces;
    LPM_Lang lang;
    LPM_Meteo meteoFormat;
} LPM_EditorParams;

uint32_t LPM_API_execEditor(const LPM_EditorParams * params);
size_t LPM_API_getHeapSize();


// -----------------------------------------------------------------------------
//  Запись карты функций поддержки в ПЗУ


#endif // LPM_EDITOR_API_H
