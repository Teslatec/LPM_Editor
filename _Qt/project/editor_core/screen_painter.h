#ifndef SCREEN_PAINTER_H
#define SCREEN_PAINTER_H

#include "modules.h"

typedef struct ScreenPainterTextTable
{
    const unicode_t * shortcurs;
    const unicode_t * textBufferFull;
    const unicode_t * clipboardFull;
} ScreenPainterTextTable;

typedef struct ScreenPainter
{
    const Modules * modules;
    LPM_UnicodeDisplay * display;
    const ScreenPainterTextTable * textTable;
} ScreenPainter;

typedef enum EditorMessage
{
    EDITOR_MESSAGE_SHORT_CUTS,
    EDITOR_MESSAGE_TEXT_BUFFER_FULL,
    EDITOR_MESSAGE_CLIPBOARD_FULL
} EditorMessage;

static inline void ScreenPainter_init
    ( ScreenPainter * o,
      const Modules * modules,
      LPM_UnicodeDisplay * display,
      const ScreenPainterTextTable * textTable )
{
    o->modules   = modules;
    o->display   = display;
    o->textTable = textTable;
}

void ScreenPainter_drawEditorMessage
    ( ScreenPainter * o,
      EditorMessage msg );

void ScreenPainter_drawEditorState();

/*
 * 1. Информационные строки
 * 2. Строка состояния текстового редактора
 * 3. Строка состояния полей области вставки (???)
 * 4. Меню создания шаблона
 * 5. Меню загрузки шаблона
 */

#endif // SCREEN_PAINTER_H
