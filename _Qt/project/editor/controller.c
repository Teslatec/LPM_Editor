#include "controller.h"

#include "command_reader.h"
#include "clipboard.h"
#include "lpm_text_storage.h"
#include "action_storage.h"
#include "page_formatter.h"
#include "core.h"
#include "lpm_lang.h"
#include "lpm_text_operator.h"

#define KEYBOARD_WATI_TIMEOUT 1000

#define KEYBOARD_BUFFER_SIZE 10
#define LINE_BUFFER_SIZE (PAGE_CHAR_AMOUNT*2)
#define ACTIONS_BUFFER_SIZE (PAGE_CHAR_AMOUNT*PAGE_LINE_AMOUNT*2)
#define CLIPBOARD_BUFFER_SIZE (ACTIONS_BUFFER_SIZE)

static unicode_t keyboardBuffer[KEYBOARD_BUFFER_SIZE];
static unicode_t lineBuffer[LINE_BUFFER_SIZE];
static unicode_t actionsBuffer[ACTIONS_BUFFER_SIZE];
static unicode_t clipboardBuffer[CLIPBOARD_BUFFER_SIZE];

static Core             core;
static CmdReader        cmdReader;
static PageFormatter    pageFormatter;
static ActionStorage    actionStorage;
static Clipboard        clipboard;
static LPM_TextStorage  textStorage;
static LPM_Lang         lang;
static LPM_TextOperator textOperator;

static Modules modules;

static void _createAndInit(const LPM_EditorParams * param);

void Controller_exec(const LPM_EditorParams * param)
{
    // Читаем настройки
    // Создаем объекты (пока что они статические и глобальные)

    // Выполняем действия при запуске

    _createAndInit(param);
    Core_exec(modules.core);

    // Выполняем действия при завершении работы

    // Очищаем память
}

void _createAndInit(const LPM_EditorParams * param)
{
    modules.keyboard = param->kbd;
    modules.display  = param->dsp;

    modules.keyboardBuffer.data = keyboardBuffer;
    modules.keyboardBuffer.size = KEYBOARD_BUFFER_SIZE;

    modules.lineBuffer.data = lineBuffer;
    modules.lineBuffer.size = LINE_BUFFER_SIZE;

    modules.actionsBuffer.data = actionsBuffer;
    modules.actionsBuffer.size = ACTIONS_BUFFER_SIZE;

    modules.clipboardBuffer.data = clipboardBuffer;
    modules.clipboardBuffer.size = CLIPBOARD_BUFFER_SIZE;

    modules.textBuffer.data = (unicode_t*)param->textBuffer->data;
    modules.textBuffer.size = param->textBuffer->size / sizeof(unicode_t);

    Core_init(&core, &modules);
    CmdReader_init(&cmdReader, param->kbd, &modules.keyboardBuffer);
    LPM_TextStorage_init(&textStorage, &modules.textBuffer);
    LPM_TextOperator_init(&textOperator, &lang);
    LPM_Lang_init(&lang, LPM_LANG_RUS_ENG);
    ActionStorage_init(&actionStorage, &modules.actionsBuffer);
    Clipboard_init(&clipboard, &modules.clipboardBuffer);
    PageFormatter_init(&pageFormatter, &modules);

    modules.core          = &core;
    modules.cmdReader     = &cmdReader;
    modules.pageFormatter = &pageFormatter;
    modules.actionStorage = &actionStorage;
    modules.clipboard     = &clipboard;
    modules.textStorage   = &textStorage;
    modules.textOperator  = &textOperator;
    modules.lang          = &lang;
}
