#ifndef EDITOR_FLAGS_H
#define EDITOR_FLAGS_H

typedef enum TextFlag
{
    TEXT_FLAG_TEXT = 0,
    TEXT_FLAG_TAB,
    TEXT_FLAG_NEW_LINE,
    TEXT_FLAG_REMOVE_NEXT_CHAR,
    TEXT_FLAG_REMOVE_PREV_CHAR,
    TEXT_FLAG_REMOVE_PAGE,
    TEXT_FLAG_TRUCATE_LINE,
} TextFlag;

typedef enum CursorFlag
{
    CURSOR_FLAG_MOVE    = 0 << 0,
    CURSOR_FLAG_SELECT  = 1 << 0,

    CURSOR_FLAG_CHAR    = 0 << 1,
    CURSOR_FLAG_LINE    = 1 << 1,
    CURSOR_FLAG_PAGE    = 2 << 1,

    CURSOR_FLAG_UP      = 0 << 3,
    CURSOR_FLAG_DOWN    = 1 << 3,
    CURSOR_FLAG_LEFT    = 2 << 3,
    CURSOR_FLAG_RIGHT   = 3 << 3,

    CURSOR_FLAG_BEGIN   = 0 << 3,
    CURSOR_FLAG_END     = 1 << 3,

    CURSOR_FLAG_PREV    = 0 << 3,
    CURSOR_FLAG_NEXT    = 1 << 3,
} CursorFlag;

#endif // EDITOR_FLAGS_H
