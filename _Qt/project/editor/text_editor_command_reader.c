#include "text_editor_command_reader.h"

typedef enum _Modifiers
{
    _MODIFIER_SHIFT  = 0x01,
    _MODIFIER_ALT    = 0x02,
    _MODIFIER_CTRL   = 0x04,
} _Modifiers;

static TextEditorCmd _processAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * rxBuf);
static TextEditorCmd _processAsNotHavingModifierAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);
static TextEditorCmd _processCtrlSequenceAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);
static TextEditorCmd _saveSymAndReturnEnteredSymCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);
static TextEditorCmd _processAsWithCtrlAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);
static TextEditorCmd _processAsWithAltAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);
static TextEditorCmd _processAsWithShiftAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);
static TextEditorCmd _processAsPureAndConvertToCmd(TextEditorCmdReader * obj, const Unicode_Buf * buf);

static void _processAsHavingModifier( TextEditorCmdReader * obj,
                                      const Unicode_Buf * buf );

static bool _firstCharIsModifier(const Unicode_Buf * buf);
static bool _thereIsNoModifiers(TextEditorCmdReader * obj);
static bool _thereIsOnlyShiftModifier(TextEditorCmdReader * obj);
static bool _firstCharIsControllingChar(const Unicode_Buf * buf);
static bool _thereIsCtrlModifier(TextEditorCmdReader * obj);
static bool _thereIsAltModifier(TextEditorCmdReader * obj);
static bool _thereIsShiftModifier(TextEditorCmdReader * obj);

void TextEditorCmdReader_init( TextEditorCmdReader * obj,
                               LPM_UnicodeKeyboard * keyboard,
                               Unicode_Buf * kbdBuf )
{
    obj->keyboard     = keyboard;
    obj->kbdBuf       = kbdBuf;
    obj->receivedSize = 0;
    obj->flags        = 0;
    obj->modifiers    = 0;
    obj->isReplacementMode = false;
}

TextEditorCmd TextEditorCmdReader_read( TextEditorCmdReader * obj,
                                        uint32_t timeoutMs )
{
    TextEditorCmd cmd = __TEXT_EDITOR_NO_CMD;
    Unicode_Buf buf =
    {
        .data = obj->kbdBuf->data,
        .size = obj->kbdBuf->size
    };

    do
    {
        LPM_UnicodeKeyboard_read(obj->keyboard, &buf, timeoutMs);
        cmd = _processAndConvertToCmd(obj, &buf);
    }
    while(cmd == __TEXT_EDITOR_NO_CMD);
    return cmd;
}

TextEditorCmd _processAndConvertToCmd( TextEditorCmdReader * obj,
                                       const Unicode_Buf * rxBuf )
{
    if(_firstCharIsModifier(rxBuf))
    {
        _processAsHavingModifier(obj, rxBuf);
        return __TEXT_EDITOR_NO_CMD;
    }
    return _processAsNotHavingModifierAndConvertToCmd(obj, rxBuf);
}

TextEditorCmd _processAsNotHavingModifierAndConvertToCmd( TextEditorCmdReader * obj,
                                                          const Unicode_Buf * buf)
{
    if(_firstCharIsControllingChar(buf))
        return _processCtrlSequenceAndConvertToCmd(obj, buf);

    if(_thereIsNoModifiers(obj))
        return _saveSymAndReturnEnteredSymCmd(obj, buf);

    if(_thereIsOnlyShiftModifier(obj))
        return _saveSymAndReturnEnteredSymCmd(obj, buf);

    return _processCtrlSequenceAndConvertToCmd(obj, buf);
}

TextEditorCmd _processCtrlSequenceAndConvertToCmd( TextEditorCmdReader * obj,
                                                   const Unicode_Buf * buf )
{
    if(_thereIsCtrlModifier(obj))
        return _processAsWithCtrlAndConvertToCmd(obj, buf);

    if(_thereIsAltModifier(obj))
        return _processAsWithAltAndConvertToCmd(obj, buf);

    else if(_thereIsShiftModifier(obj))
        return _processAsWithShiftAndConvertToCmd(obj, buf);

    return _processAsPureAndConvertToCmd(obj, buf);
}

TextEditorCmd _saveSymAndReturnEnteredSymCmd( TextEditorCmdReader * obj,
                                              const Unicode_Buf * buf )
{
    obj->flags = 0;
    obj->receivedSize = buf->size;
    return TEXT_EDITOR_CMD_ENTER_SYMBOL;
}


TextEditorCmd _processAsWithCtrlAndConvertToCmd( TextEditorCmdReader * obj,
                                                 const Unicode_Buf * buf )
{
    TextEditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case 's': cmd = TEXT_EDITOR_CMD_SAVE; break;
        case 'w': cmd = TEXT_EDITOR_CMD_CLEAR_CLIPBOARD; break;
        case 'v': cmd = TEXT_EDITOR_CMD_PASTE; break;
        case 'c': cmd = TEXT_EDITOR_CMD_COPY;  break;
        case 'x': cmd = TEXT_EDITOR_CMD_CUT;  break;
        case 'z': cmd = TEXT_EDITOR_CMD_UNDO; break;

        case 'b':
            cmd = TEXT_EDITOR_CMD_DELETE;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_END;
            break;

        case ' ':
            obj->receivedSize = 1;
            obj->kbdBuf->data[0] = UNICODE_LIGHT_SHADE;
            cmd = TEXT_EDITOR_CMD_ENTER_SYMBOL;
            break;

        case 'e':
            cmd = TEXT_EDITOR_CMD_DELETE;
            obj->flags |= TEXT_EDITOR_FLAG_PAGE;
            break;

        case '1': cmd = TEXT_EDITOR_CMD_OUTLINE_HELP;  break;
        case '?': cmd = TEXT_EDITOR_CMD_OUTLINE_STATE; break;

        case 'a':
            cmd = TEXT_EDITOR_CMD_CHANGE_SELECTION;
            obj->flags |= TEXT_EDITOR_FLAG_PAGE;
            break;

        case 'd':
            cmd = TEXT_EDITOR_CMD_CHANGE_SELECTION;
            obj->flags |= TEXT_EDITOR_FLAG_LINE;
            break;

        case UNICODE_HOME:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_PAGE | TEXT_EDITOR_FLAG_BEGIN;
            break;

        case UNICODE_END:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_PAGE | TEXT_EDITOR_FLAG_END;
            break;

        //case UNICODE_ENTER: break;
        default: cmd = __TEXT_EDITOR_NO_CMD;
    }
    return cmd;
}

TextEditorCmd _processAsWithAltAndConvertToCmd( TextEditorCmdReader * obj,
                                                const Unicode_Buf * buf )
{
    TextEditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case UNICODE_HOME:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_BEGIN;
            break;

        case UNICODE_END:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_END;
            break;

        case UNICODE_PGUP:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_PAGE | TEXT_EDITOR_FLAG_PREV;
            break;

        case UNICODE_PGDN:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_PAGE | TEXT_EDITOR_FLAG_NEXT;
            break;

        default: cmd = __TEXT_EDITOR_NO_CMD;
    }
    return cmd;
}

TextEditorCmd _processAsWithShiftAndConvertToCmd( TextEditorCmdReader * obj,
                                                  const Unicode_Buf * buf )
{
    TextEditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case UNICODE_UP:
            cmd = TEXT_EDITOR_CMD_CHANGE_SELECTION;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_PREV;
            break;

        case UNICODE_DOWN:
            cmd = TEXT_EDITOR_CMD_CHANGE_SELECTION;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_NEXT;
            break;

        case UNICODE_LEFT:
            cmd = TEXT_EDITOR_CMD_CHANGE_SELECTION;
            obj->flags |= TEXT_EDITOR_FLAG_BACKWARD;
            break;

        case UNICODE_RIGHT:
            cmd = TEXT_EDITOR_CMD_CHANGE_SELECTION;
            obj->flags |= TEXT_EDITOR_FLAG_FORWARD;
            break;

        //case UNICODE_ENTER: break;
        default: cmd = __TEXT_EDITOR_NO_CMD;
    }

    return cmd;
}

TextEditorCmd _processAsPureAndConvertToCmd( TextEditorCmdReader * obj,
                                             const Unicode_Buf * buf )
{
    TextEditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case UNICODE_TAB:
            cmd = TEXT_EDITOR_CMD_ENTER_TAB;
            break;

        case UNICODE_ENTER:
            cmd = TEXT_EDITOR_CMD_ENTER_NEW_LINE;
            break;

        case UNICODE_BACKSPACE:
            cmd = TEXT_EDITOR_CMD_DELETE;
            obj->flags |= TEXT_EDITOR_FLAG_BACKWARD;
            break;

        case UNICODE_DEL:
            cmd = TEXT_EDITOR_CMD_DELETE;
            obj->flags |= TEXT_EDITOR_FLAG_FORWARD;
            break;

        case UNICODE_ESC:
            cmd = TEXT_EDITOR_CMD_EXIT;
            break;

        case UNICODE_INSERT_A:
            cmd = TEXT_EDITOR_CMD_CHANGE_MODE;
            obj->isReplacementMode = true;
            break;

        case UNICODE_INSERT_N:
            cmd = TEXT_EDITOR_CMD_CHANGE_MODE;
            obj->isReplacementMode = false;
            break;

        //case UNICODE_LANG_A: break;
        //case UNICODE_LANG_N: break;

        case UNICODE_UP:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_PREV;
            break;

        case UNICODE_DOWN:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_LINE | TEXT_EDITOR_FLAG_NEXT;
            break;

        case UNICODE_LEFT:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_BACKWARD;
            break;

        case UNICODE_RIGHT:
            cmd = TEXT_EDITOR_CMD_MOVE_CURSOR;
            obj->flags |= TEXT_EDITOR_FLAG_FORWARD;
            break;

        default: cmd = __TEXT_EDITOR_NO_CMD;
    }
    return cmd;
}

void _processAsHavingModifier( TextEditorCmdReader * obj,
                               const Unicode_Buf * buf )
{
    uint8_t mod = obj->modifiers;
    unicode_t firstChar = buf->data[0];
    switch(firstChar)
    {
        case UNICODE_CTRL_P:  mod |=  _MODIFIER_CTRL;  break;
        case UNICODE_CTRL_R:  mod &= ~_MODIFIER_CTRL;  break;
        case UNICODE_ALT_P:   mod |=  _MODIFIER_ALT;   break;
        case UNICODE_ALT_R:   mod &= ~_MODIFIER_ALT;   break;
        case UNICODE_SHIFT_P: mod |=  _MODIFIER_SHIFT; break;
        case UNICODE_SHIFT_R: mod &= ~_MODIFIER_SHIFT; break;
        default:;
    }
    obj->modifiers = mod;
}

bool _firstCharIsModifier(const Unicode_Buf * buf)
{
    unicode_t firstChar = buf->data[0];
    firstChar &= ~(0x0080);
    return (firstChar >= 0xE101) && (firstChar <= 0xE103);
}
bool _thereIsNoModifiers(TextEditorCmdReader * obj)
{
    return obj->modifiers == 0;
}

bool _thereIsOnlyShiftModifier(TextEditorCmdReader * obj)
{
    return obj->modifiers == _MODIFIER_SHIFT;
}

bool _firstCharIsControllingChar(const Unicode_Buf * buf)
{
    unicode_t firstChar = buf->data[0];
    return Unicode_symIsCtrlSym(firstChar);
    //return Unicode_getSymType(sym) == UNICODE_SYM_TYPE_CONTROL;
}

bool _thereIsCtrlModifier(TextEditorCmdReader * obj)
{
    return obj->modifiers & _MODIFIER_CTRL;
}

bool _thereIsAltModifier(TextEditorCmdReader * obj)
{
    return obj->modifiers & _MODIFIER_ALT;
}

bool _thereIsShiftModifier(TextEditorCmdReader * obj)
{
    return obj->modifiers & _MODIFIER_SHIFT;
}
