#ifndef ACTION_STORAGE_H
#define ACTION_STORAGE_H

#include "lpm_unicode.h"
#include "lpm_structs.h"

struct LPM_TextBuffer;
struct Modules;

typedef enum Action
{
    ACTION_TEXT_ENTERED,
    ACTION_TEXT_DELETED,
    ACTION_TEXT_CHANGED
} Action;

typedef struct ActionStorage
{
    const struct Modules * modules;
    LPM_SelectionCursor savedCursor;

    Unicode_Buf * deletedText;
    LPM_SelectionCursor cursorBefore;
    uint32_t enteredTextLenght;
    uint32_t deletedTextLenght;
    uint8_t flags;
} ActionStorage;

inline void ActionStorage_init(ActionStorage * o, Unicode_Buf * textBuf)
{
    o->deletedText = textBuf;
}

bool ActionStorage_push
    ( ActionStorage * o,
      const Unicode_Buf * removingText,
      const LPM_SelectionCursor * insertArea );

bool ActionStorage_pop
    ( ActionStorage * o,
      LPM_SelectionCursor * textCursor );

//void ActionStorage_saveDeletedText
//        ( ActionStorage * obj,
//          const Unicode_Buf * deletedText,
//          const LPM_SelectionCursor * cursor );

//void ActionStorage_saveEnteredText
//        ( ActionStorage * obj,
//          uint32_t enteredTextLenght,
//          const LPM_SelectionCursor * cursor );

//void ActionStorage_saveChangedText
//        ( ActionStorage * obj,
//          const Unicode_Buf * deletedText,
//          uint32_t enteredTextLenght,
//          const LPM_SelectionCursor * cursor );

//inline Action ActionStorage_getAction(ActionStorage * obj);

//void ActionStorage_restoreDeletedText
//        ( ActionStorage * obj,
//          Unicode_Buf * deletedText,
//          LPM_SelectionCursor * cursorBefore );

//void ActionStorage_restoreEnteredText
//        ( ActionStorage * obj,
//          uint32_t * enteredTextLenght,
//          LPM_SelectionCursor * cursorBefore );

//void ActionStorage_restoreChangedText
//        ( ActionStorage * obj,
//          Unicode_Buf * deletedText,
//          uint32_t * enteredTextLenght,
//          LPM_SelectionCursor * cursor );

#endif // ACTION_STORAGE_H
