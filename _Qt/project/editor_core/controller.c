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

//#define KEYBOARD_WATI_TIMEOUT 1000
//#define KEYBOARD_BUFFER_SIZE 10
//#define LINE_BUFFER_SIZE (PAGE_CHAR_AMOUNT*2)
//#define ACTIONS_BUFFER_SIZE (PAGE_CHAR_AMOUNT*PAGE_LINE_AMOUNT*2)
//#define CLIPBOARD_BUFFER_SIZE (ACTIONS_BUFFER_SIZE)

//static unicode_t keyboardBuffer[KEYBOARD_BUFFER_SIZE];
//static unicode_t lineBuffer[LINE_BUFFER_SIZE];
//static unicode_t actionsBuffer[ACTIONS_BUFFER_SIZE];
//static unicode_t clipboardBuffer[CLIPBOARD_BUFFER_SIZE];

//static unicode_t textBuffer[23*1024];

//static Core             core;
//static CmdReader        cmdReader;
//static PageFormatter    pageFormatter;
//static TextStorage  textStorage;
//static TextStorageImpl  textStorageImpl;
//static LPM_LangFxns     langFxns;
//static TextOperator textOperator;
//static TextBuffer   clipboardTextBuffer;
//static TextBuffer   undoTextBuffer;
//static ScreenPainter    screenPainter;

extern const unicode_t * editorTextShortcut;
extern const unicode_t * editorTextTextBufferFull;
extern const unicode_t * editorTextClipboardFull;

static ScreenPainterTextTable screenPainterTextTable;

//static Modules modules;
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
            _alignSize(p->settings->charBufferSize);
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

    test_print("Sizeof Core:", sizeof(Core), _alignSize(sizeof(Core)));
    test_print("Sizeof CmdReader:", sizeof(CmdReader), _alignSize(sizeof(CmdReader)));
    test_print("Sizeof PageFormatter:", sizeof(PageFormatter), _alignSize(sizeof(PageFormatter)));
    test_print("Sizeof TextBuffer:", sizeof(TextBuffer), _alignSize(sizeof(TextBuffer)));
    test_print("Sizeof TextBuffer:", sizeof(TextBuffer), _alignSize(sizeof(TextBuffer)));
    test_print("Sizeof TextStorage:", sizeof(TextStorage), _alignSize(sizeof(TextStorage)));
    test_print("Sizeof TextStorageImpl:", sizeof(TextStorageImpl), _alignSize(sizeof(TextStorageImpl)));
    test_print("Sizeof TextOperator:", sizeof(TextOperator), _alignSize(sizeof(TextOperator)));
    test_print("Sizeof ScreenPainter:", sizeof(ScreenPainter), _alignSize(sizeof(ScreenPainter)));
    test_print("Sizeof LPM_LangFxns:", sizeof(LPM_LangFxns), _alignSize(sizeof(LPM_LangFxns)));
    test_print("Sizeof LPM_EncodingFxns:", sizeof(LPM_EncodingFxns), _alignSize(sizeof(LPM_EncodingFxns)));
    test_print("Sizeof LPM_MeteoFxns:", sizeof(LPM_MeteoFxns), _alignSize(sizeof(LPM_MeteoFxns)));

    test_print("Aligned desired heap size n got size:", Controller_calcDesiredHeapSize(sp), sp->settings->heap.size);

    // Проверить размер кучи
    if(Controller_calcDesiredHeapSize(sp) > sp->settings->heap.size)
        return NULL;

    // Разместить Modules
    Modules * m = (Modules*)sp->settings->heap.data;

    test_print("Modules:", (uint32_t)m, _alignSize(sizeof(Modules)));

    // Разместить Объекты
    uint32_t moduleAddr = (uint32_t)(&m->core);
    uint32_t heapAddr = (uint32_t)(m) + _alignSize(sizeof(Modules));
    for(int i = 0; i < MODULES_AMOUNT; i++)
    {
        test_print("Curr module", moduleAddr, heapAddr);
        *((uint32_t*)moduleAddr) = heapAddr;
        heapAddr += _alignSize(moduleSizeTable[i]);
        moduleAddr += POINTER_SIZE;
    }

    // Разместить буферы
    m->lineBuffer.data = (unicode_t*)heapAddr;
    m->lineBuffer.size = _alignSize(sp->settings->lineBufferSize);
    test_print("lineBuffer", (uint32_t)&m->lineBuffer, heapAddr);
    test_print("lineBuffer", heapAddr, m->lineBuffer.size);
    heapAddr += m->lineBuffer.size;

    m->charBuffer.data = (unicode_t*)heapAddr;
    m->charBuffer.size = _alignSize(sp->settings->charBufferSize);
    test_print("charBuffer", (uint32_t)&m->charBuffer, heapAddr);
    test_print("charBuffer", heapAddr, m->charBuffer.size);

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

    PageFormatter_init(m->pageFormatter, m, sp->displayDriver);

    screenPainterTextTable.shortcurs      = editorTextShortcut;
    screenPainterTextTable.textBufferFull = editorTextTextBufferFull;
    screenPainterTextTable.clipboardFull  = editorTextClipboardFull;
    ScreenPainter_init(m->screenPainter, m, sp->displayDriver, &screenPainterTextTable);
}

size_t _alignSize(size_t size)
{
    return (size + 3) & (~3);
}

//void _createAndInit
//        ( const LPM_EditorUserParams * userParams,
//          const LPM_EditorSystemParams * systemParams )
//{
//    modules.keyboard = systemParams->keyboardDriver;
//    modules.display  = systemParams->displayDriver;

//    modules.lineBuffer.data = lineBuffer;
//    modules.lineBuffer.size = LINE_BUFFER_SIZE;

//    Unicode_Buf tmp;

//    Core_init(&core, &modules, LPM_END_OF_LINE_TYPE_CRLF);

//    tmp.data = keyboardBuffer;
//    tmp.size = KEYBOARD_BUFFER_SIZE;
//    CmdReader_init(&cmdReader, modules.keyboard, &tmp);

////    tmp.data = (unicode_t*)param->textBuffer->data;
////    tmp.size = param->textBuffer->size / sizeof(unicode_t);
//    tmp.data = textBuffer;
//    tmp.size = systemParams->settings->textBuffer.size;
//    TextStorageImpl_init(&textStorageImpl, &tmp);

//    TextStorage_init(&textStorage, &textStorageImpl);

//    LPM_SupportFxns fxns;
//    fxns.lang = &langFxns;
//    (*systemParams->readSupportFxnsFxn)(&fxns, userParams->lang);

//    tmp.data = (unicode_t*)specChars;
//    tmp.size = 1;
//    TextOperator_init(&textOperator, &langFxns, &tmp);


//    tmp.data = clipboardBuffer;
//    tmp.size = CLIPBOARD_BUFFER_SIZE;
//    TextBuffer_init(&clipboardTextBuffer, &tmp, &modules);

//    tmp.data = actionsBuffer;
//    tmp.size = ACTIONS_BUFFER_SIZE;
//    TextBuffer_init(&undoTextBuffer, &tmp, &modules);

//    PageFormatter_init(&pageFormatter, &modules);

//    screenPainterTextTable.shortcurs = editorTextShortcut;
//    screenPainterTextTable.textBufferFull = editorTextTextBufferFull;
//    screenPainterTextTable.clipboardFull = editorTextClipboardFull;
//    ScreenPainter_init(&screenPainter, &modules, &screenPainterTextTable);

//    modules.core                = &core;
//    modules.cmdReader           = &cmdReader;
//    modules.pageFormatter       = &pageFormatter;
//    modules.clipboardTextBuffer = &clipboardTextBuffer;
//    modules.undoTextBuffer      = &undoTextBuffer;
//    modules.textStorage         = &textStorage;
//    modules.textOperator        = &textOperator;
//    modules.screenPainter       = &screenPainter;
//}
