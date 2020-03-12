#include "command_reader.h"

typedef CmdReader Obj;
typedef EditorCmd Cmd;
typedef Unicode_Buf UBuf;

typedef enum _Modifiers
{
    _MODIFIER_SHIFT  = 0x01,
    _MODIFIER_ALT    = 0x02,
    _MODIFIER_CTRL   = 0x04,
} _Modifiers;

static Cmd _processAndConvertToCmd(Obj * obj, const UBuf * rxBuf);
static Cmd _processAsNotHavingModifierAndConvertToCmd(Obj * obj, const UBuf * buf);
static Cmd _processCtrlSequenceAndConvertToCmd(Obj * obj, const UBuf * buf);
static Cmd _saveCharsAndReturnTextChangedCmd(Obj * obj, const UBuf * buf);
static Cmd _processAsWithCtrlAndConvertToCmd(Obj * obj, const UBuf * buf);
static Cmd _processAsWithAltAndConvertToCmd(Obj * obj, const UBuf * buf);
static Cmd _processAsWithShiftAndConvertToCmd(Obj * obj, const UBuf * buf);
static Cmd _processAsPureAndConvertToCmd(Obj * obj, const UBuf * buf);

static void _processAsHavingModifier(Obj * obj, const UBuf * buf);

static bool _firstCharIsModifier(const UBuf * buf);
static bool _thereIsNoModifiers(Obj * obj);
static bool _thereIsOnlyShiftModifier(Obj * obj);
static bool _firstCharIsControllingChar(const UBuf * buf);
static bool _thereIsCtrlModifier(Obj * obj);
static bool _thereIsAltModifier(Obj * obj);
static bool _thereIsShiftModifier(Obj * obj);

void CmdReader_init
        ( CmdReader * obj,
          LPM_UnicodeKeyboard * keyboard,
          const Unicode_Buf * kbdBuf,
          uint16_t timeout)
{
    obj->keyboard     = keyboard;
    obj->kbdBuf.data  = kbdBuf->data;
    obj->kbdBuf.size  = kbdBuf->size;
    obj->receivedSize = 0;
    obj->timeout      = timeout;
    obj->flags        = 0;
    obj->modifiers    = 0;
    obj->isReplacementMode = false;
}

EditorCmd CmdReader_read(CmdReader * obj)
{
    EditorCmd cmd = __EDITOR_NO_CMD;
    Unicode_Buf buf = { obj->kbdBuf.data, obj->kbdBuf.size };
    do
    {
        LPM_UnicodeKeyboard_read(obj->keyboard, &buf, obj->timeout);
        cmd = _processAndConvertToCmd(obj, &buf);
    }
    while(cmd == __EDITOR_NO_CMD);
    return cmd;
}

Cmd _processAndConvertToCmd(Obj * obj, const UBuf * rxBuf)
{
    if(_firstCharIsModifier(rxBuf))
    {
        _processAsHavingModifier(obj, rxBuf);
        return __EDITOR_NO_CMD;
    }
    return _processAsNotHavingModifierAndConvertToCmd(obj, rxBuf);
}

Cmd _processAsNotHavingModifierAndConvertToCmd(Obj * obj, const UBuf * buf)
{
    if(_firstCharIsControllingChar(buf))
        return _processCtrlSequenceAndConvertToCmd(obj, buf);

    if(_thereIsNoModifiers(obj))
        return _saveCharsAndReturnTextChangedCmd(obj, buf);

    if(_thereIsOnlyShiftModifier(obj))
        return _saveCharsAndReturnTextChangedCmd(obj, buf);

    return _processCtrlSequenceAndConvertToCmd(obj, buf);
}

Cmd _processCtrlSequenceAndConvertToCmd(Obj * obj, const UBuf * buf)
{
    if(_thereIsCtrlModifier(obj))
        return _processAsWithCtrlAndConvertToCmd(obj, buf);

    if(_thereIsAltModifier(obj))
        return _processAsWithAltAndConvertToCmd(obj, buf);

    else if(_thereIsShiftModifier(obj))
        return _processAsWithShiftAndConvertToCmd(obj, buf);

    return _processAsPureAndConvertToCmd(obj, buf);
}

Cmd _saveCharsAndReturnTextChangedCmd(Obj * obj, const UBuf * buf)
{
    obj->flags = TEXT_FLAG_TEXT;
    obj->receivedSize = buf->size;
    return EDITOR_CMD_TEXT_CHANGED;
}

Cmd _processAsWithCtrlAndConvertToCmd(Obj * obj, const UBuf * buf)
{
    Cmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case 's':
            cmd = EDITOR_CMD_SAVE;
            break;

        case 'w':
            cmd = EDITOR_CMD_CLEAR_CLIPBOARD;
            break;

        case 'v':
            cmd = EDITOR_CMD_PASTE;
            break;

        case 'c':
            cmd = EDITOR_CMD_COPY;
            break;

        case 'x':
            cmd = EDITOR_CMD_CUT;
            break;

        case 'z':
            cmd = EDITOR_CMD_UNDO;
            break;

        case 'b':
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_TRUCATE_LINE;
            break;

        case ' ':
            obj->receivedSize = 1;
            //obj->kbdBuf.data[0] = UNICODE_LIGHT_SHADE;
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_INSERTION_BORDER;//TEXT_FLAG_TEXT;
            break;

        case 'e':
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_REMOVE_PAGE;
            break;

        case '1':
            cmd = EDITOR_CMD_OUTLINE_HELP;
            break;

        case '?':
            cmd = EDITOR_CMD_OUTLINE_STATE;
            break;

        case 'a':
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_SELECT |
                    CURSOR_FLAG_PAGE ;
            break;

        case 'd':
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_SELECT |
                    CURSOR_FLAG_LINE;
            break;

        case UNICODE_LEFT: //UNICODE_HOME:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_PAGE |
                    CURSOR_FLAG_BEGIN ;
            break;

        case UNICODE_RIGHT://UNICODE_END:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_PAGE |
                    CURSOR_FLAG_END ;
            break;

        //case UNICODE_ENTER: break;
        default: cmd = __EDITOR_NO_CMD;
    }
    return cmd;
}

Cmd _processAsWithAltAndConvertToCmd(Obj * obj, const UBuf * buf)
{
    EditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case UNICODE_LEFT: //UNICODE_HOME:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_LINE |
                    CURSOR_FLAG_BEGIN;
            break;

        case UNICODE_RIGHT: //UNICODE_END:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_LINE |
                    CURSOR_FLAG_END;
            break;

        case UNICODE_UP: //UNICODE_PGUP:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_PAGE |
                    CURSOR_FLAG_PREV;
            break;

        case UNICODE_DOWN: //UNICODE_PGDN:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_PAGE |
                    CURSOR_FLAG_NEXT;
            break;

        default: cmd = __EDITOR_NO_CMD;
    }
    return cmd;
}

Cmd _processAsWithShiftAndConvertToCmd(Obj * obj, const UBuf * buf)
{
    EditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case UNICODE_UP:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_SELECT |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_UP;
            break;

        case UNICODE_DOWN:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_SELECT |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_DOWN;
            break;

        case UNICODE_LEFT:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_SELECT |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_LEFT;
            break;

        case UNICODE_RIGHT:
            cmd = EDITOR_CMD_CURSOR_CHANGED;
            obj->flags =
                    CURSOR_FLAG_SELECT |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_RIGHT;
            break;

        //case UNICODE_ENTER: break;
        default: cmd = __EDITOR_NO_CMD;
    }

    return cmd;
}

Cmd _processAsPureAndConvertToCmd(Obj * obj, const UBuf * buf)
{
    EditorCmd cmd;
    unicode_t firstChar = buf->data[0];
    obj->flags = 0;
    obj->receivedSize = 0;

    switch(firstChar)
    {
        case UNICODE_TAB:
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_TAB;
            break;

        case UNICODE_ENTER:
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_NEW_LINE;
            break;

        case UNICODE_BACKSPACE:
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_REMOVE_PREV_CHAR;
            break;

        case UNICODE_DEL:
            cmd = EDITOR_CMD_TEXT_CHANGED;
            obj->flags = TEXT_FLAG_REMOVE_NEXT_CHAR;
            break;

        case UNICODE_ESC:
            cmd = EDITOR_CMD_EXIT;
            break;

        case UNICODE_INSERT_A:
            cmd = EDITOR_CMD_CHANGE_MODE;
            obj->isReplacementMode = true;
            break;

        case UNICODE_INSERT_N:
            cmd = EDITOR_CMD_CHANGE_MODE;
            obj->isReplacementMode = false;
            break;

        //case UNICODE_LANG_A: break;
        //case UNICODE_LANG_N: break;

        case UNICODE_UP:
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_UP;
            break;

        case UNICODE_DOWN:
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_DOWN;
            break;

        case UNICODE_LEFT:
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_LEFT;
            break;

        case UNICODE_RIGHT:
            obj->flags =
                    CURSOR_FLAG_MOVE |
                    CURSOR_FLAG_CHAR |
                    CURSOR_FLAG_RIGHT;
            break;

        default: cmd = __EDITOR_NO_CMD;
    }
    return cmd;
}

void _processAsHavingModifier(Obj * obj, const UBuf * buf)
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

bool _firstCharIsModifier(const UBuf * buf)
{
    unicode_t firstChar = buf->data[0];
    firstChar &= ~(0x0080);
    return (firstChar >= 0xE101) && (firstChar <= 0xE103);
}
bool _thereIsNoModifiers(Obj * obj)
{
    return obj->modifiers == 0;
}

bool _thereIsOnlyShiftModifier(Obj * obj)
{
    return obj->modifiers == _MODIFIER_SHIFT;
}

bool _firstCharIsControllingChar(const UBuf * buf)
{
    unicode_t firstChar = buf->data[0];
    return Unicode_symIsCtrlSym(firstChar);
}

bool _thereIsCtrlModifier(Obj * obj)
{
    return obj->modifiers & _MODIFIER_CTRL;
}

bool _thereIsAltModifier(Obj * obj)
{
    return obj->modifiers & _MODIFIER_ALT;
}

bool _thereIsShiftModifier(Obj * obj)
{
    return obj->modifiers & _MODIFIER_SHIFT;
}
