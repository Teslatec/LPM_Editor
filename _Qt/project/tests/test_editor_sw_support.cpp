#include "test_editor_sw_support.h"
#include <QDebug>

extern "C"
{

#include "lpm_lang_api.h"
#include "lpm_meteo_api.h"
#include "lpm_gui_texts_api.h"
#include "lang_rus_eng.h"

}

static const size_t TEXT_BUFFER_SIZE = 1024*1024;
static const size_t UNDO_BUFFER_SIZE = 4096;
static const size_t CLIPBOARD_SIZE = 4096;
static const size_t INSERTIONS_BUFFER_SIZE = 4096;
static const size_t RECOVERY_BUFFER_SIZE = 4096;
static const size_t HEAP_SIZE = 1200;

static uint32_t textBuffer[TEXT_BUFFER_SIZE/4];
static uint32_t undoBuffer[UNDO_BUFFER_SIZE/4];
static uint32_t clipboard[CLIPBOARD_SIZE/4];
static uint32_t insertionsBuffer[INSERTIONS_BUFFER_SIZE/4];
static uint32_t recoveryBuffer[RECOVERY_BUFFER_SIZE/4];
static uint32_t heap[HEAP_SIZE/4];

static const size_t MAX_METEO_SIZE = 14000;
static const size_t MAX_FAX_CHAIN_SIZE = 14000;
static const size_t MAX_TEMPLATE_SIZE = 16128;
static const size_t MAX_TEMPLATE_AMOUNT = 16;
static const size_t TEMPLATE_BAD_NAME_AMOUNT = 2;
static const uint16_t templateBadNameTable[TEMPLATE_BAD_NAME_AMOUNT] =
{ 0x0000, 0xFFFF };

static const size_t LINE_BUFFER_SIZE = 256;
static const size_t CHAR_BUFFER_SIZE = 10;
static const size_t COPY_BUFFER_SIZE = 256;
static const size_t SCREEN_CHAR_AMOUNT = 64;
static const size_t SCREEN_LINE_AMOUNT = 16;
static const size_t PAGE_IN_GROUP_AMOUNT = 32;
static const size_t PAGE_GROUP_AMOUNT = 32;
static const unicode_t INSERTION_BORDER_CHAR = UNICODE_LIGHT_SHADE;
static const LPM_EndlType DEFAULT_END_OF_LINE_TYPE = LPM_ENDL_TYPE_CRLF;
static const LPM_insertionInputPolicy INSERTION_INPUT_POLICY = LPM_INSERTION_INPUT_POLICY_NO_INPUT;
static const uint32_t KEYBOARD_TIMEOUT = 1000;
static const uint8_t TAB_SPACE_AMOUNT = 5;

static const LPM_EditorSettings editorSettings =
{
    { (uint8_t*)textBuffer,       TEXT_BUFFER_SIZE       },
    { (uint8_t*)undoBuffer,       UNDO_BUFFER_SIZE       },
    { (uint8_t*)clipboard,        CLIPBOARD_SIZE         },
    { (uint8_t*)insertionsBuffer, INSERTIONS_BUFFER_SIZE },
    { (uint8_t*)recoveryBuffer,   RECOVERY_BUFFER_SIZE   },
    { (uint8_t*)heap, HEAP_SIZE },
    MAX_METEO_SIZE,
    MAX_FAX_CHAIN_SIZE,
    MAX_TEMPLATE_SIZE,
    MAX_TEMPLATE_AMOUNT,
    templateBadNameTable,
    TEMPLATE_BAD_NAME_AMOUNT,
    KEYBOARD_TIMEOUT,
    LINE_BUFFER_SIZE,
    CHAR_BUFFER_SIZE,
    COPY_BUFFER_SIZE,
    { SCREEN_CHAR_AMOUNT,
      SCREEN_LINE_AMOUNT,
      PAGE_IN_GROUP_AMOUNT,
      PAGE_GROUP_AMOUNT },
    INSERTION_BORDER_CHAR,
    DEFAULT_END_OF_LINE_TYPE,
    INSERTION_INPUT_POLICY,
    TAB_SPACE_AMOUNT
};

bool TestEditorSwSupport::readSettings(LPM_EditorSettings * setting)
{
    *setting = editorSettings;
    qDebug() << (int)textBuffer;
    return true;
}

bool TestEditorSwSupport::readSupportFxns(LPM_SupportFxns * fxns, LPM_Lang lang)
{
    auto meteoCheckFormat = [](const Unicode_Buf * msgBuffer, LPM_Meteo format)
    {
        Q_UNUSED(msgBuffer);
        qDebug() << "Проверяю формат метеосообщения:" << format;
        return true;
    };

    auto fromMeteo = [](const Unicode_Buf * msgBuffer, LPM_Meteo format)
    {
        Q_UNUSED(msgBuffer);
        qDebug() << "Преобразую метеосообщение формата" << format << "в текст";
    };

    auto toMeteo = [](const Unicode_Buf * msgBuffer, LPM_Meteo format)
    {
        Q_UNUSED(msgBuffer);
        qDebug() << "Преобразую текст в метеосообщение формата" << format;
    };

    auto encodingCheckText = [](const LPM_Buf * text, LPM_Encoding encoding, size_t maxTextSize, bool ignoreSpecChars)
    {
        Q_UNUSED(text);
        qDebug() << "Проверяю текст размером:" << maxTextSize
                 << "на соотвествие кодировке:" << encoding << "спец. символы"
                 << (ignoreSpecChars ? "считаю текстом" : "не считаю текстом");
        return true;
    };

    auto encodingFromUnicode = [](const LPM_Buf * text, LPM_Encoding encoding)
    {
        Q_UNUSED(text);
        qDebug() << "Перекодирую из кодировки" << encoding << "в UCS2-LE";
    };

    auto encodingToUnicode = [](const LPM_Buf * text, LPM_Encoding encoding)
    {
        Q_UNUSED(text);
        qDebug() << "Перекодирую из UCS2-LE в кодировку" << encoding;
    };

    fxns->meteo->checkFormat = meteoCheckFormat;
    fxns->meteo->fromMeteo   = fromMeteo;
    fxns->meteo->toMeteo     = toMeteo;

    fxns->encoding->checkText   = encodingCheckText;
    fxns->encoding->toUnicode   = encodingToUnicode;
    fxns->encoding->fromUnicode = encodingFromUnicode;

    if(lang == LPM_LANG_RUS_ENG)
    {
        fxns->lang->checkInputChar = &Lang_RusEng_checkInputChar;
        fxns->lang->nextChar       = &Lang_RusEng_nextChar;
        fxns->lang->prevChar       = &Lang_RusEng_prevChar;
        return true;
    }
    return false;
}

bool TestEditorSwSupport::readGuiText(Unicode_Buf * text, LPM_Lang lang, LPM_GuiTextId id)
{
    (void)text;
    (void)lang;
    (void)id;
    return true;
}
