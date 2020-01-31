#ifndef TEXT_EDITOR_COMMAND_READER_H
#define TEXT_EDITOR_COMMAND_READER_H

#include "lpm_unicode_keyboard.h"

typedef enum TextEditorCmd
{
    TEXT_EDITOR_CMD_MOVE_CURSOR,
    TEXT_EDITOR_CMD_CHANGE_SELECTION,
    TEXT_EDITOR_CMD_ENTER_SYMBOL,
    TEXT_EDITOR_CMD_ENTER_TAB,
    TEXT_EDITOR_CMD_ENTER_NEW_LINE,
    TEXT_EDITOR_CMD_DELETE,
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
    __TEXT_EDITOR_NO_CMD
} TextEditorCmd;

typedef enum TextEditorFlag
{
    TEXT_EDITOR_FLAG_FORWARD  = 0x01,
    TEXT_EDITOR_FLAG_BACKWARD = 0x02,
    TEXT_EDITOR_FLAG_BEGIN    = 0x04,
    TEXT_EDITOR_FLAG_END      = 0x08,
    TEXT_EDITOR_FLAG_NEXT     = 0x10,
    TEXT_EDITOR_FLAG_PREV     = 0x20,
    TEXT_EDITOR_FLAG_PAGE     = 0x40,
    TEXT_EDITOR_FLAG_LINE     = 0x80,
} TextEditorFlag;

typedef struct TextEditorCmdReader
{
    Unicode_Buf kbdBuf;
    size_t receivedSize;
    LPM_UnicodeKeyboard * keyboard;
    uint16_t  flags;
    bool      isReplacementMode;
    uint8_t   modifiers;
} TextEditorCmdReader;

void TextEditorCmdReader_init( TextEditorCmdReader * obj,
                               LPM_UnicodeKeyboard * keyboard,
                               const Unicode_Buf * kbdBuf);

TextEditorCmd TextEditorCmdReader_read( TextEditorCmdReader * obj,
                                        uint32_t timeoutMs );

bool TextEditorCmdReader_errorOccured(TextEditorCmdReader * obj);

inline void TextEditorCmdReader_getText( TextEditorCmdReader * obj,
                                         Unicode_Buf * buf )
{
    buf->data = obj->kbdBuf.data;
    buf->size = obj->receivedSize;
}

inline uint8_t TextEditorCmdReader_getFlags(TextEditorCmdReader * obj)
{
    return obj->flags;
}

inline bool TextEditorCmdReader_isReplacementMode(TextEditorCmdReader * obj)
{
    return obj->isReplacementMode;
}

#endif // TEXT_EDITOR_COMMAND_READER_H
