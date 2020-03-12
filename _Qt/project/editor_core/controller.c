#include "controller.h"

#include "command_reader.h"
#include "text_storage.h"
#include "page_formatter.h"
#include "core.h"
#include "lpm_lang_api.h"
#include "lpm_encoding_api.h"
#include "lpm_meteo_api.h"
#include "text_operator.h"
#include "text_buffer.h"
#include "screen_painter.h"
#include "lang_rus_eng.h"

#include <string.h>

extern const unicode_t * editorTextShortcut;
extern const unicode_t * editorTextTextBufferFull;
extern const unicode_t * editorTextClipboardFull;

static ScreenPainterTextTable screenPainterTextTable;
static const unicode_t specChars[1] = { UNICODE_LIGHT_SHADE };

static Modules * _allocateModules(const LPM_EditorSystemParams * sp);
static void _clearHeap(const LPM_EditorSystemParams * sp);
static void _initModules
        ( Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp);
static size_t _alignSize(size_t size);

uint32_t Controller_exec
        ( const LPM_EditorUserParams * userParams,
          const LPM_EditorSystemParams * systemParams )
{
    // Создаем объекты (пока что они статические и глобальные)
    Modules * modules = _allocateModules(systemParams);
    if(modules == NULL)
        return LPM_EDITOR_ERROR_BAD_HEAP_SIZE;

    _initModules(modules, userParams, systemParams);

    // Выполняем действия при запуске

    Core_exec(modules->core);

    // Выполняем действия при завершении работы

    // Очищаем память
    _clearHeap(systemParams);

    return 0;
}


size_t Controller_calcDesiredHeapSize(const LPM_EditorSystemParams * p)
{
    return
            _alignSize(sizeof( Modules          )) +
            _alignSize(sizeof( Core             )) +
            _alignSize(sizeof( CmdReader        )) +
            _alignSize(sizeof( PageFormatter    )) +
            _alignSize(sizeof( TextBuffer       )) +
            _alignSize(sizeof( TextBuffer       )) +
            _alignSize(sizeof( TextStorage      )) +
            _alignSize(sizeof( TextStorageImpl  )) +
            _alignSize(sizeof( TextOperator     )) +
            _alignSize(sizeof( ScreenPainter    )) +
            _alignSize(sizeof( LPM_LangFxns     )) +
            _alignSize(sizeof( LPM_EncodingFxns )) +
            _alignSize(sizeof( LPM_MeteoFxns    )) +
            _alignSize(p->settings->lineBufferSize) +
            _alignSize(p->settings->charBufferSize) +
            _alignSize(p->settings->pageParams.lineAmount * sizeof(LineMap)) +
            _alignSize(p->settings->pageParams.pageGroupAmount * sizeof(size_t) );
}

Modules * _allocateModules(const LPM_EditorSystemParams * sp)
{
    static const size_t moduleSizeTable[MODULES_AMOUNT] =
    {
        sizeof(Core),
        sizeof(CmdReader),
        sizeof(PageFormatter),
        sizeof(TextBuffer),
        sizeof(TextBuffer),
        sizeof(TextStorage),
        sizeof(TextStorageImpl),
        sizeof(TextOperator),
        sizeof(ScreenPainter),
        sizeof(LPM_LangFxns),
        sizeof(LPM_EncodingFxns),
        sizeof(LPM_MeteoFxns),
    };

    // Проверить размер кучи
    if(Controller_calcDesiredHeapSize(sp) > sp->settings->heap.size)
        return NULL;

    // Разместить Modules
    Modules * m = (Modules*)sp->settings->heap.data;

    // Разместить Объекты
    uint32_t moduleAddr = (uint32_t)(&m->core);
    uint32_t heapAddr = (uint32_t)(m) + _alignSize(sizeof(Modules));
    for(int i = 0; i < MODULES_AMOUNT; i++)
    {
        *((uint32_t*)moduleAddr) = heapAddr;
        heapAddr += _alignSize(moduleSizeTable[i]);
        moduleAddr += POINTER_SIZE;
    }

    // Разместить буферы
    size_t alignedSize = _alignSize(sp->settings->lineBufferSize);
    m->lineBuffer.data = (unicode_t*)heapAddr;
    m->lineBuffer.size = alignedSize / sizeof(unicode_t);
    heapAddr += alignedSize;

    alignedSize = _alignSize(sp->settings->charBufferSize);
    m->charBuffer.data = (unicode_t*)heapAddr;
    m->charBuffer.size = alignedSize / sizeof(unicode_t);
    heapAddr += alignedSize;

    // Разместить таблицы карт строк и базовых адресов групп страниц
    m->pageGroupBaseTable = (size_t*)heapAddr;
    heapAddr += _alignSize(sp->settings->pageParams.pageGroupAmount * sizeof(size_t));

    m->lineMapTable = (LineMap*)heapAddr;

    return m;
}

void _clearHeap(const LPM_EditorSystemParams * sp)
{
    memset(sp->settings->heap.data, 0, _alignSize(sp->settings->heap.size));
}

void _initModules(Modules * m, const LPM_EditorUserParams * up, const LPM_EditorSystemParams * sp)
{
    Unicode_Buf tmp;

    Core_init(m->core, m, sp->displayDriver, up->endOfLineType);

    CmdReader_init(m->cmdReader, sp->keyboardDriver, &m->charBuffer);

    tmp.data = (unicode_t*)sp->settings->textBuffer.data;
    tmp.size = sp->settings->textBuffer.size;
    TextStorageImpl_init(m->textStorageImpl, &tmp);
    TextStorage_init(m->textStorage, m->textStorageImpl);

    LPM_SupportFxns fxns;
    fxns.lang     = m->langFxns;
    fxns.encoding = m->encodingFxns;
    fxns.meteo    = m->meteoFxns;
    LPM_API_readSupportFxns(sp, &fxns, up->lang);

    tmp.data = (unicode_t*)specChars;
    tmp.size = 1;
    TextOperator_init(m->textOperator, m->langFxns, &tmp);

    tmp.data = (unicode_t*)sp->settings->clipboard.data;
    tmp.size = sp->settings->clipboard.size;
    TextBuffer_init(m->clipboardTextBuffer, &tmp, m);

    tmp.data = (unicode_t*)sp->settings->undoBuffer.data;
    tmp.size = sp->settings->undoBuffer.size;
    TextBuffer_init(m->undoTextBuffer, &tmp, m);

    PageFormatter_init(m->pageFormatter, m, sp->displayDriver, &sp->settings->pageParams);

    screenPainterTextTable.shortcurs      = editorTextShortcut;
    screenPainterTextTable.textBufferFull = editorTextTextBufferFull;
    screenPainterTextTable.clipboardFull  = editorTextClipboardFull;
    ScreenPainter_init(m->screenPainter, m, &sp->settings->pageParams, sp->displayDriver, &screenPainterTextTable);
}

size_t _alignSize(size_t size)
{
    return (size + 3) & (~3);
}
