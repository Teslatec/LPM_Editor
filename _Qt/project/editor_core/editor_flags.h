#ifndef EDITOR_FLAGS_H
#define EDITOR_FLAGS_H

typedef enum TextFlag
{
    TEXT_FLAG_TEXT = 0,
    TEXT_FLAG_TAB,
    TEXT_FLAG_NEW_LINE,
    TEXT_FLAG_INSERTION_BORDER,
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

    CURSOR_FLAG_BEGIN   = 0 << 5,
    CURSOR_FLAG_END     = 1 << 5,
    CURSOR_FLAG_PREV    = 2 << 5,
    CURSOR_FLAG_NEXT    = 3 << 5,
} CursorFlag;

#define CURSOR_TYPE_FIELD       0x01
#define CURSOR_GOAL_FIELD       0x06
#define CURSOR_DIRECTION_FIELD  0x18
#define CURSOR_BORDER_FIELD     0x60

#endif // EDITOR_FLAGS_H
