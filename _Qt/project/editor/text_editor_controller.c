#include "text_editor_controller.h"

#include "text_editor_command_reader.h"
#include "text_editor_clipboard.h"
#include "text_editor_text_operator.h"
#include "text_editor_user_actions.h"
#include "text_editor_page_formatter.h"
#include "text_editor_core.h"

#define KEYBOARD_WATI_TIMEOUT 1000

#define CMD_READER_KBDBUF_SIZE 10

static TextEditorCmdReader cmdReader;
static TextEditorTextOperator textOperator;
//static TextEditorClipboard clipboard;
//static TextEditorActionStorage actionStorage;
//static TextEditorPageFormatter pageFormatter;
//static TextEditorCore core;

static unicode_t cmdReaderKbdBuf[CMD_READER_KBDBUF_SIZE];

void TextEditorControler_exec(const LPM_EditorParams * param)
{
    // Читаем настройки
    // Создаем объекты (пока что они статические и глобальные)
    // Выполняем действия при запуске

    Unicode_Buf kbdBuf = { cmdReaderKbdBuf, CMD_READER_KBDBUF_SIZE };
    TextEditorCmdReader_init(&cmdReader, param->kbd, &kbdBuf);

    // Выполняем алгоритм редактирования
    for(;;)
    {
        TextEditorCmd cmd;
        cmd = TextEditorCmdReader_read(&cmdReader, KEYBOARD_WATI_TIMEOUT);

        uint16_t flags = TextEditorCmdReader_getFlags(&cmdReader);
        TextEditorCmdReader_getText(&cmdReader, &kbdBuf);
        bool mode = TextEditorCmdReader_isReplacementMode(&cmdReader);

        if(cmd == TEXT_EDITOR_CMD_ENTER_SYMBOL)
            for(size_t i = 0; i < kbdBuf.size; i++)
                test_command_reader(cmd, flags, kbdBuf.data[i], mode);
        else
            test_command_reader(cmd, flags, kbdBuf.data[0], mode);

        if(cmd == TEXT_EDITOR_CMD_EXIT)
            break;
    }

    Unicode_Buf buf =
    {
        .data = (unicode_t*)param->textBuffer->data,
        .size = param->textBuffer->size / 2
    };
    TextEditorTextOperator_init(&textOperator, &buf);

    Unicode_Buf rb =
    {
        .data = buf.data + 30,
        .size = 20,
    };
    TextEditorTextOperator_read(&textOperator, 0, &rb);
    param->textBuffer->size = rb.size;

    // Выполняем действия при завершении работы
    // Очищаем память
}
