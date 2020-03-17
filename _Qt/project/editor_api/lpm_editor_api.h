#ifndef LPM_EDITOR_API_H
#define LPM_EDITOR_API_H

#include "lpm_unicode_keyboard.h"
#include "lpm_unicode_display.h"
#include "lpm_file.h"
#include "lpm_lang_api.h"
#include "lpm_encoding_api.h"
#include "lpm_meteo_api.h"
#include "lpm_gui_texts_api.h"

/* -----------------------------------------------------------------------------
 * Указатели на фукнции программной поддержки. Эти функции должен определить
 *  пользователь и передать в программу редактора как параметры.
 */

struct LPM_EditorSettings;
struct LPM_SupportFxns;

typedef bool (*LPM_API_readSettingsFxn)(struct LPM_EditorSettings*);
typedef bool (*LPM_API_readSupportFxnsFxn)(struct LPM_SupportFxns*, LPM_Lang);
typedef bool (*LPM_API_readGuiTextFxn)(Unicode_Buf*, LPM_Lang, LPM_GuiTextId);

/* -----------------------------------------------------------------------------
 * Константы-перечисления
 */

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
    LPM_EDITOR_MODE_TEXT_NEW      = 0x00,
    LPM_EDITOR_MODE_TEXT_EDIT     = 0x01,
    LPM_EDITOR_MODE_TEXT_VIEW     = 0x02,
    LPM_EDITOR_MODE_METEO_NEW     = 0x10,
    LPM_EDITOR_MODE_METEO_EDIT    = 0x11,
    LPM_EDITOR_MODE_METEO_VIEW    = 0x12,
    LPM_EDITOR_MODE_TEMPLATE_NEW  = 0x20,
    LPM_EDITOR_MODE_TEMPLATE_EDIT = 0x21,
    LPM_EDITOR_MODE_TEMPLATE_VIEW = 0x22,
    LPM_EDITOR_MODE_TEMP_INS_NEW  = 0x30,
    LPM_EDITOR_MODE_TEMP_INS_EDIT = 0x31,
    LPM_EDITOR_MODE_TEMP_INS_VIEW = 0x32
} LPM_EditorMode;

typedef enum LPM_EditorResult
{
    // Ошибки
    LPM_EDITOR_ERROR_BAD_ENCODING         = (1u << 31),
    LPM_EDITOR_ERROR_NO_PLACE_TO_PRINT    = (1u << 29),
    LPM_EDITOR_ERROR_DISPLAY              = (1u << 28),
    LPM_EDITOR_ERROR_KEYBOARD             = (1u << 27),
    LPM_EDITOR_ERROR_FLASH_READ           = (1u << 26),
    LPM_EDITOR_ERROR_FLASH_WRITE          = (1u << 25),
    LPM_EDITOR_ERROR_BAD_METEO_FORMAT     = (1u << 24),
    LPM_EDITOR_ERROR_BAD_TEMPLATE_FORMAT  = (1u << 23),
    LPM_EDITOR_ERROR_BAD_TEMPLATE_NAME    = (1u << 22),
    LPM_EDITOR_ERROR_BAD_INSERTION_FORMAT = (1u << 21),
    LPM_EDITOR_ERROR_BAD_HEAP_SIZE        = (1u << 20),
    // Предупреждения
    LPM_EDITOR_WARNING_DIFF_ENDLS   = (1u <<  1),
    LPM_EDITOR_WARNING_BAD_CHAR_SEQ = (1u <<  0),
    // ОК
    LPM_EDITOR_OK = 0,
    LPM_EDITOR_CANCELED_BY_USER = (1u << 2),
} LPM_EditorResult;

typedef enum LPM_insertionInputPolicy
{
    LPM_INSERTION_INPUT_POLICY_NO_INPUT,
    LPM_INSERTION_INPUT_POLICY_BEGIN_ONLY,
    LPM_INSERTION_INPUT_POLICY_END_ONLY,
    LPM_INSERTION_INPUT_POLICY_BEGIN_END
} LPM_insertionInputPolicy;

/* -----------------------------------------------------------------------------
 * Вспомогательные структуры параметров
 */

/*
 * Структура настроек. Это - "константные" настройки редактора. Предполагается,
 *  что они будут храниться в ПЗУ. Не путать с LPM_EditorUserParams!
 */

typedef struct LPM_EditorPageParams
{
    uint16_t charAmount;
    uint16_t lineAmount;
    uint16_t pageGroupAmount;
    uint16_t pageInGroupAmount;
} LPM_EditorPageParams;

typedef struct LPM_EditorSettings
{
    LPM_Buf textBuffer;
    LPM_Buf undoBuffer;
    LPM_Buf clipboard;
    LPM_Buf insertionsBuffer;
    LPM_Buf encodingBuffer; // ??
    LPM_Buf heap;
    size_t maxMeteoSize;
    size_t maxFaxChainSize;
    size_t maxTemplateSize;
    uint16_t maxTemplateAmount;
    uint16_t keyboardTimeout;
    uint16_t lineBufferSize;
    uint16_t charBufferSize;
    LPM_EditorPageParams pageParams;
    unicode_t insertionBorderChar;
    LPM_EndOfLineType defaultEndOfLineType;
    LPM_insertionInputPolicy insertionInputPolicy;
    uint8_t tabSpaceAmount;
} LPM_EditorSettings;

/*
 * Структура указателей на структуры, которые в свою очередь содержат указатели
 *  на функции определенного типа: функции поддержки языков, кодировок и работы
 *  с метеосообщениями.
 */

typedef struct LPM_SupportFxns
{
    LPM_LangFxns     * lang;
    LPM_EncodingFxns * encoding;
    LPM_MeteoFxns    * meteo;
} LPM_SupportFxns;

typedef struct LPM_EditorSystemParams
{
    LPM_UnicodeDisplay  * displayDriver;
    LPM_UnicodeKeyboard * keyboardDriver;
    LPM_File * templatesFile;
    LPM_EditorSettings * settings;
    LPM_API_readSupportFxnsFxn readSupportFxnsFxn;
    LPM_API_readGuiTextFxn     readGuiTextFxn;
} LPM_EditorSystemParams;

typedef struct LPM_EditorUserParams
{
    LPM_EditorMode mode;
    LPM_EndOfLineType endOfLineType;
    LPM_CursorInitPos cursorInitPos;
    LPM_Encoding encodingFrom;
    LPM_Encoding encodingTo;
    bool prepareToPrint;
    uint8_t lineBeginSpaces;
    LPM_Lang lang;
    LPM_Meteo meteoFormat;
} LPM_EditorUserParams;

uint32_t LPM_API_execEditor
        ( const LPM_EditorUserParams * userParams,
          const LPM_EditorSystemParams * systemParams );

size_t LPM_API_getDesiredHeapSize
        (const LPM_EditorSystemParams * systemParams);

static inline bool LPM_API_readSupportFxns
        ( const LPM_EditorSystemParams * sp,
          LPM_SupportFxns * fxns,
          LPM_Lang lang )
{
    return (*sp->readSupportFxnsFxn)(fxns, lang);
}

static inline bool LPM_API_readGuiText
        ( LPM_EditorSystemParams * sp,
          Unicode_Buf * text,
          LPM_Lang lang,
          LPM_GuiTextId id )
{
    return (*sp->readGuiTextFxn)(text, lang, id);
}

#endif // LPM_EDITOR_API_H