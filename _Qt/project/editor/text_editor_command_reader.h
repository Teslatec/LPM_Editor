#ifndef TEXT_EDITOR_COMMAND_READER_H
#define TEXT_EDITOR_COMMAND_READER_H

#include "lpm_unicode_keyboard.h"
#include "text_editor_flags.h"

typedef enum TextEditorCmd
{
    TEXT_EDITOR_CMD_CURSOR_CHANGED,
    TEXT_EDITOR_CMD_TEXT_CHANGED,
    TEXT_EDITOR_CMD_CHANGE_MODE,
    TEXT_EDITOR_CMD_COPY,
    TEXT_EDITOR_CMD_PASTE,
    TEXT_EDITOR_CMD_CUT,
    TEXT_EDITOR_CMD_SAVE,
    TEXT_EDITOR_CMD_CLEAR_CLIPBOARD,
    TEXT_EDITOR_CMD_UNDO,
    TEXT_EDITOR_CMD_EXIT,
    TEXT_EDITOR_CMD_OUTLINE_HELP,
    TEXT_EDITOR_CMD_OUTLINE_STATE,
    TEXT_EDITOR_TIMEOUT,
    __TEXT_EDITOR_NO_CMD
} TextEditorCmd;

typedef struct TextEditorCmdReader
{
    LPM_UnicodeKeyboard * keyboard;
    Unicode_Buf * kbdBuf;
    size_t receivedSize;
    uint16_t flags;
    uint8_t modifiers;
    bool isReplacementMode;
} TextEditorCmdReader;

void TextEditorCmdReader_init
        ( TextEditorCmdReader * obj,
          LPM_UnicodeKeyboard * keyboard,
          Unicode_Buf * kbdBuf );

TextEditorCmd TextEditorCmdReader_read
                ( TextEditorCmdReader * obj,
                  uint32_t timeoutMs );

bool TextEditorCmdReader_errorOccured(TextEditorCmdReader * obj);

inline void TextEditorCmdReader_getText
        ( TextEditorCmdReader * obj,
          Unicode_Buf * buf )
{
    buf->data = obj->kbdBuf->data;
    buf->size = obj->receivedSize;
}

inline uint8_t TextEditorCmdReader_getFlags
        (TextEditorCmdReader * obj) { return obj->flags; }

inline bool TextEditorCmdReader_isReplacementMode
        (TextEditorCmdReader * obj) { return obj->isReplacementMode; }

#endif // TEXT_EDITOR_COMMAND_READER_H
