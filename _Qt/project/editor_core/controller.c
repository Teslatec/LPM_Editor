#include "controller.h"

#include "command_reader.h"
#include "text_storage.h"
#include "page_formatter.h"
#include "core.h"
#include "lpm_lang_api.h"
#include "text_operator.h"
#include "text_buffer.h"
#include "screen_painter.h"
#include "lang_rus_eng.h"

#define KEYBOARD_WATI_TIMEOUT 1000

#define KEYBOARD_BUFFER_SIZE 10
#define LINE_BUFFER_SIZE (PAGE_CHAR_AMOUNT*2)
#define ACTIONS_BUFFER_SIZE (PAGE_CHAR_AMOUNT*PAGE_LINE_AMOUNT*2)
#define CLIPBOARD_BUFFER_SIZE (ACTIONS_BUFFER_SIZE)

static unicode_t keyboardBuffer[KEYBOARD_BUFFER_SIZE];
static unicode_t lineBuffer[LINE_BUFFER_SIZE];
static unicode_t actionsBuffer[ACTIONS_BUFFER_SIZE];
static unicode_t clipboardBuffer[CLIPBOARD_BUFFER_SIZE];

static unicode_t textBuffer[23*1024];

static Core             core;
static CmdReader        cmdReader;
static PageFormatter    pageFormatter;
static TextStorage  textStorage;
static TextStorageImpl  textStorageImpl;
static LPM_LangFxns     langFxns;
static TextOperator textOperator;
static TextBuffer   clipboardTextBuffer;
static TextBuffer   undoTextBuffer;
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
    size_t fullSize = sizeof(unicode_t)*(LINE_BUFFER_SIZE + KEYBOARD_BUFFER_SIZE) +
            sizeof(Core) + sizeof(CmdReader) + sizeof(PageFormatter) +
            sizeof(TextStorageImpl) + sizeof(TextStorage) +
            sizeof(LPM_LangFxns) + sizeof(TextOperator) +
            sizeof(TextBuffer)*2 + sizeof(ScreenPainter) +
            sizeof(ScreenPainterTextTable) + sizeof(Modules);

    test_print("Full internal RAM size:", fullSize, 0);

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
    modules.keyboard = param->keyboard;
    modules.display  = param->display;

    modules.lineBuffer.data = lineBuffer;
    modules.lineBuffer.size = LINE_BUFFER_SIZE;

    Unicode_Buf tmp;

    Core_init(&core, &modules, LPM_END_OF_LINE_TYPE_CRLF);

    tmp.data = keyboardBuffer;
    tmp.size = KEYBOARD_BUFFER_SIZE;
    CmdReader_init(&cmdReader, param->keyboard, &tmp);

//    tmp.data = (unicode_t*)param->textBuffer->data;
//    tmp.size = param->textBuffer->size / sizeof(unicode_t);
    tmp.data = textBuffer;
    tmp.size = 23*1024;
    TextStorageImpl_init(&textStorageImpl, &tmp);

    TextStorage_init(&textStorage, &textStorageImpl);

    //LPM_Lang_init(&langFxns, LPM_LANG_RUS_ENG);
    langFxns.checkInputChar = &Lang_RusEng_checkInputChar;
    langFxns.nextChar       = &Lang_RusEng_nextChar;
    langFxns.prevChar       = &Lang_RusEng_prevChar;

    tmp.data = (unicode_t*)specChars;
    tmp.size = 1;
    TextOperator_init(&textOperator, &langFxns, &tmp);


    tmp.data = clipboardBuffer;
    tmp.size = CLIPBOARD_BUFFER_SIZE;
    TextBuffer_init(&clipboardTextBuffer, &tmp, &modules);

    tmp.data = actionsBuffer;
    tmp.size = ACTIONS_BUFFER_SIZE;
    TextBuffer_init(&undoTextBuffer, &tmp, &modules);

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
    modules.textOperator        = &textOperator;
    modules.screenPainter       = &screenPainter;
}
