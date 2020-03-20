#ifndef COMMAND_READER_H
#define COMMAND_READER_H

#include "lpm_unicode_keyboard.h"
#include "editor_flags.h"
#include "lpm_editor_api.h"

typedef enum EditorCmd
{
    EDITOR_CMD_CURSOR_CHANGED,
    EDITOR_CMD_TEXT_CHANGED,
    EDITOR_CMD_CHANGE_MODE,
    EDITOR_CMD_COPY,
    EDITOR_CMD_PASTE,
    EDITOR_CMD_CUT,
    EDITOR_CMD_SAVE,
    EDITOR_CMD_CLEAR_CLIPBOARD,
    EDITOR_CMD_UNDO,
    EDITOR_CMD_EXIT,
    EDITOR_CMD_OUTLINE_HELP,
    EDITOR_CMD_OUTLINE_STATE,
    EDITOR_CMD_TIMEOUT,
    __EDITOR_NO_CMD
} EditorCmd;

typedef struct CmdReader
{
    LPM_UnicodeKeyboard * keyboard;
    Unicode_Buf kbdBuf;
    size_t receivedSize;
    unicode_t insertionBorderChar;
    uint16_t timeout;
    uint16_t flags;
    uint8_t modifiers;
    bool isReplacementMode;
} CmdReader;

void CmdReader_init
        ( CmdReader * obj,
          const Unicode_Buf * kbdBuf,
          const LPM_EditorSystemParams * sp );

EditorCmd CmdReader_read(CmdReader * obj);

bool CmdReader_errorOccured(CmdReader * obj);

inline void CmdReader_getText
        ( CmdReader * obj,
          Unicode_Buf * buf )
{
    buf->data = obj->kbdBuf.data;
    buf->size = obj->receivedSize;
}

inline uint8_t CmdReader_getFlags
        (CmdReader * obj) { return obj->flags; }

inline bool CmdReader_isReplacementMode
        (CmdReader * obj) { return obj->isReplacementMode; }

#endif // COMMAND_READER_H
