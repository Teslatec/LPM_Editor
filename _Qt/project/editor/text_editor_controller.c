#include "text_editor_controller.h"

#include "text_editor_command_reader.h"
#include "text_editor_clipboard.h"
#include "text_editor_text_operator.h"
#include "text_editor_user_actions.h"
#include "text_editor_page_formatter.h"
#include "text_editor_core.h"

#define KEYBOARD_WATI_TIMEOUT 1000

#define KEYBOARD_BUFFER_SIZE 10
#define LINE_BUFFER_SIZE (TEXT_EDITOR_PAGE_CHAR_AMOUNT*2)
#define ACTIONS_BUFFER_SIZE (TEXT_EDITOR_PAGE_CHAR_AMOUNT*TEXT_EDITOR_PAGE_LINE_AMOUNT*2)
#define CLIPBOARD_BUFFER_SIZE (ACTIONS_BUFFER_SIZE)

static unicode_t keyboardBuffer[KEYBOARD_BUFFER_SIZE];
static unicode_t lineBuffer[LINE_BUFFER_SIZE];
static unicode_t actionsBuffer[ACTIONS_BUFFER_SIZE];
static unicode_t clipboardBuffer[CLIPBOARD_BUFFER_SIZE];

static TextEditorCore          core;
static TextEditorCmdReader     cmdReader;
static TextEditorTextOperator  textOperator;
static TextEditorTextStorage   textStorage;
static TextEditorPageFormatter pageFormatter;
static TextEditorActionStorage actionStorage;
static TextEditorClipboard     clipboard;

static TextEditorModules modules;

static void _createAndInit(const LPM_EditorParams * param);
static void _test();

void TextEditorController_exec(const LPM_EditorParams * param)
{
    // Читаем настройки
    // Создаем объекты (пока что они статические и глобальные)

    // Выполняем действия при запуске

    _createAndInit(param);
    TextEditorCore_exec(modules.core);

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

    TextEditorCore_init(&core, &modules);
    TextEditorCmdReader_init(&cmdReader, param->kbd, &modules.keyboardBuffer);
    TextEditorTextOperator_init(&textOperator, &textStorage);
    TextEditorTextStorage_init(&textStorage, &modules.textBuffer);
    TextEditorActionStorage_init(&actionStorage, &modules.actionsBuffer);
    TextEditorClipboard_init(&clipboard, &modules.clipboardBuffer);
    TextEditorPageFormatter_init(&pageFormatter, &modules);

    modules.core          = &core;
    modules.cmdReader     = &cmdReader;
    modules.textOperator  = &textOperator;
    modules.textStorage   = &textStorage;
    modules.pageFormatter = &pageFormatter;
    modules.actionStorage = &actionStorage;
    modules.clipboard     = &clipboard;
}

extern Unicode_Buf testDisplayBfrs[16];

void _test()
{
    int lineIndex = 0;
    int linePos = 0;
    LPM_DisplayCursor cursor = {{0,0},{0,0}};

    // Выполняем алгоритм редактирования
    for(;;)
    {
        TextEditorCmd cmd;
        cmd = TextEditorCmdReader_read(&cmdReader, KEYBOARD_WATI_TIMEOUT);

        uint16_t flags = TextEditorCmdReader_getFlags(&cmdReader);
        bool mode = TextEditorCmdReader_isReplacementMode(&cmdReader);

        if(cmd == TEXT_EDITOR_CMD_TEXT_CHANGED)
            for(size_t i = 0; i < modules.keyboardBuffer.size; i++)
                test_command_reader(cmd, flags, modules.keyboardBuffer.data[i], mode);
        else
            test_command_reader(cmd, flags, modules.keyboardBuffer.data[0], mode);

        Unicode_Buf lineBuf = { testDisplayBfrs[lineIndex].data, testDisplayBfrs[lineIndex].size - linePos*2 };
        LPM_Point p = { linePos, lineIndex };

        if(linePos < 32)
        {
            LPM_UnicodeDisplay_writeLine(modules.display, &lineBuf, &p);
            LPM_UnicodeDisplay_setCursor(modules.display, &cursor);
            if(++lineIndex == 16)
            {
                lineIndex = 0;
                linePos++;
            }
        }
        else
        {
            linePos = 0;
            LPM_UnicodeDisplay_clearScreen(modules.display);
        }

        if(++cursor.end.x == 64)
        {
            cursor.end.x = 0;
            if(++cursor.end.y == 0)
                cursor.end.y = 0;
        }

        if(cmd == TEXT_EDITOR_CMD_EXIT)
            break;
    }

    Unicode_Buf rb =
    {
        .data = modules.textBuffer.data + 30,
        .size = 20,
    };
    TextEditorTextOperator_read(&textOperator, 0, &rb);
}
