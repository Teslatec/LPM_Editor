#include "controller.h"

#include "command_reader.h"
#include "lpm_text_storage.h"
#include "page_formatter.h"
#include "core.h"
#include "lpm_lang.h"
#include "lpm_text_operator.h"
#include "lpm_text_buffer.h"
#include "screen_painter.h"

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
static LPM_TextStorage  textStorage;
static TextStorageImpl  textStorageImpl;
static LPM_Lang         lang;
static LPM_TextOperator textOperator;
static LPM_TextBuffer   clipboardTextBuffer;
static LPM_TextBuffer   undoTextBuffer;
static ScreenPainter    screenPainter;

extern const unicode_t * editorTextShortcut;
extern const unicode_t * editorTextTextBufferFull;
extern const unicode_t * editorTextClipboardFull;

static ScreenPainterTextTable screenPainterTextTable;

static Modules modules;
static const unicode_t specChars[1] = { UNICODE_LIGHT_SHADE };

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

    modules.lineBuffer.data = lineBuffer;
    modules.lineBuffer.size = LINE_BUFFER_SIZE;

    Unicode_Buf tmp;

    Core_init(&core, &modules, LPM_END_OF_LINE_TYPE_CRLF);

    tmp.data = keyboardBuffer;
    tmp.size = KEYBOARD_BUFFER_SIZE;
    CmdReader_init(&cmdReader, param->kbd, &tmp);

    tmp.data = (unicode_t*)param->textBuffer->data;
    tmp.size = param->textBuffer->size / sizeof(unicode_t);
    TextStorageImpl_init(&textStorageImpl, &tmp);

    LPM_TextStorage_init(&textStorage, &textStorageImpl);

    tmp.data = (unicode_t*)specChars;
    tmp.size = 1;
    LPM_TextOperator_init(&textOperator, &lang, &tmp);

    LPM_Lang_init(&lang, LPM_LANG_RUS_ENG);

    tmp.data = clipboardBuffer;
    tmp.size = CLIPBOARD_BUFFER_SIZE;
    LPM_TextBuffer_init(&clipboardTextBuffer, &tmp, &modules);

    tmp.data = actionsBuffer;
    tmp.size = ACTIONS_BUFFER_SIZE;
    LPM_TextBuffer_init(&undoTextBuffer, &tmp, &modules);

    PageFormatter_init(&pageFormatter, &modules);

    screenPainterTextTable.shortcurs = editorTextShortcut;
    screenPainterTextTable.textBufferFull = editorTextTextBufferFull;
    screenPainterTextTable.clipboardFull = editorTextClipboardFull;
    ScreenPainter_init(&screenPainter, &modules, &screenPainterTextTable);

    modules.core                = &core;
    modules.cmdReader           = &cmdReader;
    modules.pageFormatter       = &pageFormatter;
    modules.clipboardTextBuffer = &clipboardTextBuffer;
    modules.undoTextBuffer      = &undoTextBuffer;
    modules.textStorage         = &textStorage;
    modules.textStorageImpl     = &textStorageImpl;
    modules.textOperator        = &textOperator;
    modules.lang                = &lang;
    modules.screenPainter       = &screenPainter;
}
