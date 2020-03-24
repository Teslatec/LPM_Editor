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
#include "template_loader.h"

#include <string.h>

extern const unicode_t * editorTextShortcut;
extern const unicode_t * editorTextTextBufferFull;
extern const unicode_t * editorTextClipboardFull;

static ScreenPainterTextTable screenPainterTextTable;

static bool _insufficientHeapSize(const LPM_EditorSystemParams * sp);
static Modules * _allocateModules(const LPM_EditorSystemParams * sp);
static void _clearHeap(const LPM_EditorSystemParams * sp);
static void _initModules
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp);
static size_t _alignSize(size_t size);

static void _clearServiceBuffers(const LPM_EditorSystemParams * sp);
static void _clearTextBuffer(LPM_Buf * textBuffer);

static bool _modeIsOneOfTemplatesModes(LPM_EditorMode mode);
static bool _modeIsOneOfInsertionsModes(LPM_EditorMode mode);
static bool _modeIsOneOfMeteoModes(LPM_EditorMode mode);

//static bool _modeIsOneOfCreationModes(LPM_EditorMode mode);
//static bool _modeIsOneOfViewModes(LPM_EditorMode mode);

static uint32_t _execEditorInTextOrMeteoMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp );

static uint32_t _execEditorInTemplateMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp );

static uint32_t _execEditorInInsertionsMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp );



static uint32_t _prepareEditorInTextOrMeteoMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp );



static uint32_t _shutDownEditorInTextOrMeteoMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp );


static uint32_t _checkTextAndTranscodeToUnicodeIfOk
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp ,
          size_t maxSize );

static uint32_t _transformToPrintFormat
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp );

static uint32_t _createNewTemplateByGui
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint8_t * templateIndex );

static uint32_t _loadExistedTemplateByGui
        (const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint8_t * templateIndex );

static uint32_t _loadExistedTemplateByNameFromInsertionsText
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp,
          uint8_t * templateIndex );

static uint32_t _saveTemplate
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint8_t templateIndex );

static void _copyInsertionToTextBuffer(const Modules * m);
static void _copyInsertionToInsertionsBuffer(const Modules * m);

static uint32_t _execEditor(const Modules * m);


uint32_t Controller_exec
        ( const LPM_EditorUserParams * userParams,
          const LPM_EditorSystemParams * systemParams )
{
    if(_insufficientHeapSize(systemParams))
        return LPM_EDITOR_ERROR_BAD_HEAP_SIZE;

    Modules * modules = _allocateModules(systemParams);
    _clearServiceBuffers(systemParams);
    _initModules(modules, userParams, systemParams);

    uint32_t result;

    if(_modeIsOneOfTemplatesModes(userParams->mode))
        result = _execEditorInTemplateMode(modules, userParams, systemParams);
    else if(_modeIsOneOfInsertionsModes(userParams->mode))
        result = _execEditorInInsertionsMode(modules, userParams, systemParams);
    else
        result = _execEditorInTextOrMeteoMode(modules, userParams, systemParams);

    _clearServiceBuffers(systemParams);
    _clearHeap(systemParams);

    return result;
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
            _alignSize(p->settings->copyBufferSize) +
            _alignSize(p->settings->maxTemplateAmount * sizeof(uint16_t)) +
            _alignSize(p->settings->pageParams.lineAmount * sizeof(LineMap)) +
            _alignSize(p->settings->pageParams.pageGroupAmount * sizeof(size_t) );
}

bool _insufficientHeapSize(const LPM_EditorSystemParams * sp)
{
    return Controller_calcDesiredHeapSize(sp) > sp->settings->heap.size;
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
        sizeof(TextBuffer),
        sizeof(TextStorage),
        sizeof(TextStorageImpl),
        sizeof(TextOperator),
        sizeof(ScreenPainter),
        sizeof(LPM_LangFxns),
        sizeof(LPM_EncodingFxns),
        sizeof(LPM_MeteoFxns),
    };

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

    alignedSize = _alignSize(sp->settings->copyBufferSize);
    m->copyBuffer.data = (unicode_t*)heapAddr;
    m->copyBuffer.size = alignedSize / sizeof(unicode_t);
    heapAddr += alignedSize;

    // Разместить таблицы карт строк и базовых адресов групп страниц
    m->templateNameTable = (uint16_t*)heapAddr;
    heapAddr += _alignSize(sp->settings->maxTemplateAmount * sizeof(uint16_t));

    m->pageGroupBaseTable = (size_t*)heapAddr;
    heapAddr += _alignSize(sp->settings->pageParams.pageGroupAmount * sizeof(size_t));

    m->lineMapTable = (LineMap*)heapAddr;

    return m;
}

void _clearHeap(const LPM_EditorSystemParams * sp)
{
    memset(sp->settings->heap.data, 0, _alignSize(sp->settings->heap.size));
}

void _initModules
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    Unicode_Buf tmp;

    Core_init(m->core, m, sp);

    CmdReader_init(m->cmdReader, &m->charBuffer, sp);

    tmp.data = (unicode_t*)sp->settings->textBuffer.data;
    tmp.size = sp->settings->textBuffer.size / sizeof(unicode_t);
    TextStorageImpl_init(m->textStorageImpl, &tmp);
    TextStorage_init(m->textStorage, m);

    LPM_SupportFxns fxns;
    fxns.lang     = m->langFxns;
    fxns.encoding = m->encodingFxns;
    fxns.meteo    = m->meteoFxns;
    LPM_API_readSupportFxns(sp, &fxns, up->lang);

    TextOperator_init(m->textOperator, m->langFxns);

    tmp.data = (unicode_t*)sp->settings->clipboard.data;
    tmp.size = sp->settings->clipboard.size / sizeof(unicode_t);
    TextBuffer_init(m->clipboardTextBuffer, &tmp, m);

    tmp.data = (unicode_t*)sp->settings->undoBuffer.data;
    tmp.size = sp->settings->undoBuffer.size / sizeof(unicode_t);
    TextBuffer_init(m->undoTextBuffer, &tmp, m);

    tmp.data = (unicode_t*)sp->settings->recoveryBuffer.data;
    tmp.size = sp->settings->recoveryBuffer.size / sizeof(unicode_t);
    TextBuffer_init(m->recoveryBuffer, &tmp, m);

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

void _clearServiceBuffers(const LPM_EditorSystemParams * sp)
{
    memset( sp->settings->undoBuffer.data,       0, sp->settings->undoBuffer.size       );
    memset( sp->settings->clipboard.data,        0, sp->settings->clipboard.size        );
    memset( sp->settings->insertionsBuffer.data, 0, sp->settings->insertionsBuffer.size );
    memset( sp->settings->recoveryBuffer.data,   0, sp->settings->recoveryBuffer.size   );
}

void _clearTextBuffer(LPM_Buf * textBuffer)
{
    memset(textBuffer->data, 0, textBuffer->size);
}

bool _modeIsOneOfTemplatesModes(LPM_EditorMode mode)
{
    return (mode == LPM_EDITOR_MODE_TEMPLATE_NEW) ||
            (mode == LPM_EDITOR_MODE_TEMPLATE_EDIT);
}

bool _modeIsOneOfInsertionsModes(LPM_EditorMode mode)
{
    // Поскольку режим просмотра шаблонов - это на самом деле режим создания
    //  вставок, но без сохранения текста вставок, значение
    //  LPM_EDITOR_MODE_TEMPLATE_VIEW относится к режиму вставок
    return (mode == LPM_EDITOR_MODE_TEMPLATE_VIEW) ||
            (((uint8_t)mode & 0x30) == 0x30);
}

bool _modeIsOneOfMeteoModes(LPM_EditorMode mode)
{
    return ((uint8_t)mode & 0x30) == 0x10;
}

//bool _modeIsOneOfCreationModes(LPM_EditorMode mode)
//{
//    return ((uint8_t)mode & 0x03) == 0x00;
//}

//bool _modeIsOneOfViewModes(LPM_EditorMode mode)
//{
//    return ((uint8_t)mode & 0x03) == 0x02;
//}

uint32_t _execEditorInTextOrMeteoMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    uint32_t result = LPM_EDITOR_OK;

    if( up->mode == LPM_EDITOR_MODE_TEXT_NEW ||
        up->mode == LPM_EDITOR_MODE_METEO_NEW )
        TextStorage_clear(m->textStorage, true);
    else
        result = _prepareEditorInTextOrMeteoMode(m, up, sp);

    if(result != LPM_EDITOR_OK)
        return result;

    if( up->mode == LPM_EDITOR_MODE_TEXT_VIEW ||
        up->mode == LPM_EDITOR_MODE_METEO_VIEW )
        Core_setReadOnly(m->core);

    result = _execEditor(m);

    result |= _shutDownEditorInTextOrMeteoMode(m, up, sp);
    return result;
}

uint32_t _execEditorInTemplateMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    uint8_t templateIndex;
    uint32_t result = up->mode == LPM_EDITOR_MODE_TEMPLATE_NEW ?
                _createNewTemplateByGui(m, sp, &templateIndex) :
                _loadExistedTemplateByGui(m, sp, &templateIndex);

    if(result != LPM_EDITOR_OK)
        return result;

    TextStorageImpl_setMaxSize(m->textStorageImpl, sp->settings->maxTemplateSize/sizeof(unicode_t));
    Core_setTemplateMode(m->core);

    do
    {
        result = _execEditor(m);
        if(result != LPM_EDITOR_OK)
            return result;

        uint32_t badPageMap;
        Core_checkTemplateFormat(m->core, &badPageMap);
        if(badPageMap != 0)
        {
            // Выводим на экран информацию об ошибках и просим пользователя
            //  выбрать, исправить их или завершить работу. Если функция
            //  _outlineTemplateFormateErrors вернет LPM_EDITOR_OK, это значит,
            //  что пользователь согласился исправить ошибки
            result = ScreenPainter_drawTemplateFormatErrors(m->screenPainter, badPageMap);
            if(result == LPM_EDITOR_OK)
                continue;

            // Если пользователь не согласился, то функция вернет ошибку
            //  LPM_EDITOR_ERROR_BAD_TEMPLATE_FORMAT
            return result;
        }
    } while(false);

    result = _saveTemplate(m, sp, templateIndex);
    return result;
}

uint32_t _execEditorInInsertionsMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    // Режим просмотра шаблона = режим создания вставки в шаблон с запретом
    //  сохранения вставок
    bool dontSaveInsertions = up->mode == LPM_EDITOR_MODE_TEMPLATE_VIEW;
    uint32_t result;
    uint8_t templateIndex;

    if( up->mode == LPM_EDITOR_MODE_TEMP_INS_EDIT ||
        up->mode == LPM_EDITOR_MODE_TEMP_INS_VIEW )
        result = _loadExistedTemplateByNameFromInsertionsText(m, up, sp, &templateIndex);
    else
        result = _loadExistedTemplateByGui(m, sp, &templateIndex);

    if(result != LPM_EDITOR_OK)
        return result;

    Core_setInsertionsMode(m->core);
    if(up->mode == LPM_EDITOR_MODE_TEMP_INS_VIEW)
        Core_setReadOnly(m->core);

    result = Core_exec(m->core);

    if(dontSaveInsertions)
    {
        _clearTextBuffer(&sp->settings->textBuffer);
        return result;
    }

    // Текст вставок уже сформирован модулем Core и лежит в буфере вставок
    _copyInsertionToTextBuffer(m);
    LPM_Encoding_encodeTo( m->encodingFxns,
                           &sp->settings->textBuffer,
                           up->encodingTo );
    return result;
}


uint32_t _prepareEditorInTextOrMeteoMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    size_t maxTextSize = sp->settings->textBuffer.size;
    if(_modeIsOneOfMeteoModes(up->mode))
    {
        if(!LPM_Meteo_checkFormat( m->meteoFxns,
                                   &sp->settings->textBuffer,
                                   up->meteoFormat ))
            return LPM_EDITOR_ERROR_BAD_METEO_FORMAT;

        LPM_Meteo_decodeFrom( m->meteoFxns,
                              &sp->settings->textBuffer,
                              up->meteoFormat );
        maxTextSize = sp->settings->maxMeteoSize;
    }

    return _checkTextAndTranscodeToUnicodeIfOk(m, up, sp, maxTextSize);
}




uint32_t _shutDownEditorInTextOrMeteoMode
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    if(up->prepareToPrint)
    {
        uint32_t result = _transformToPrintFormat(m, up, sp);
        if(result != LPM_EDITOR_OK)
            return result;
    }

    LPM_Encoding_encodeTo( m->encodingFxns,
                           &sp->settings->textBuffer,
                           up->encodingTo );

    if(_modeIsOneOfMeteoModes(up->mode))
    {
        LPM_Meteo_encodeTo( m->meteoFxns,
                            &sp->settings->textBuffer,
                            up->meteoFormat );

        if(!LPM_Meteo_checkFormat( m->meteoFxns,
                                   &sp->settings->textBuffer,
                                   up->meteoFormat ))
            return LPM_EDITOR_ERROR_BAD_METEO_FORMAT;
    }

    return LPM_EDITOR_OK;
}

uint32_t _checkTextAndTranscodeToUnicodeIfOk
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp,
          size_t maxSize )
{
    if(!LPM_Encoding_checkText( m->encodingFxns,
                                &sp->settings->textBuffer,
                                up->encodingFrom,
                                maxSize ))
        return LPM_EDITOR_ERROR_BAD_ENCODING;

    LPM_Encoding_decodeFrom( m->encodingFxns,
                             &sp->settings->textBuffer,
                             up->encodingFrom );

    return LPM_EDITOR_OK;
}

uint32_t _transformToPrintFormat
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp )
{
    // TODO: написать преобразование в формат для печати
    (void)m; (void)up; (void)sp;
    return LPM_EDITOR_OK;
}

uint32_t _createNewTemplateByGui
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint8_t * templateIndex )
{
    uint32_t result;

    result = TemplateLoader_readTemplateNames(m, sp);
    if(result != LPM_EDITOR_OK)
        return result;

    result = ScreenPainter_createTemplateName(m->screenPainter, templateIndex);
    if(result != LPM_EDITOR_OK)
        return result;

    TextStorage_clear(m->textStorage, true);

    return result;
}

uint32_t _loadExistedTemplateByGui
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint8_t * templateIndex )
{
    uint32_t result;

    result = TemplateLoader_readTemplateNames(m, sp);
    if(result != LPM_EDITOR_OK)
        return result;

    result = ScreenPainter_selectTemplateName(m->screenPainter, templateIndex);
    if(result != LPM_EDITOR_OK)
        return result;

    return TemplateLoader_readText(m, sp, *templateIndex);
}

uint32_t _loadExistedTemplateByNameFromInsertionsText
        ( const Modules * m,
          const LPM_EditorUserParams * up,
          const LPM_EditorSystemParams * sp,
          uint8_t * templateIndex )
{
    uint32_t result;
    uint16_t templateName;

    result = TemplateLoader_readTemplateNames(m, sp);
    if(result != LPM_EDITOR_OK)
        return result;

    result = _checkTextAndTranscodeToUnicodeIfOk(m, up, sp, sp->settings->insertionsBuffer.size);
    if(result != LPM_EDITOR_OK)
        return result;

    if(!Core_checkInsertionFormatAndReadNameIfOk(m->core, &templateName))
        return LPM_EDITOR_ERROR_BAD_INSERTION_FORMAT;

    result = TemplateLoader_findTemplateIndexByName(m, sp, templateName, templateIndex);
    if(result != LPM_EDITOR_OK)
        return result;

    _copyInsertionToInsertionsBuffer(m);

    return TemplateLoader_readText(m, sp, *templateIndex);
}

uint32_t _saveTemplate
        ( const Modules * m,
          const LPM_EditorSystemParams * sp,
          uint8_t templateIndex )
{
    uint32_t result = TemplateLoader_saveText(m, sp, templateIndex);
    if(result != LPM_EDITOR_OK)
        return result;

    _clearTextBuffer(&sp->settings->textBuffer);
    return result;
}

void _copyInsertionToTextBuffer(const Modules * m)
{
    (void)m;
    // TODO: копирование вставок в буфер текста
}

void _copyInsertionToInsertionsBuffer(const Modules * m)
{
    (void)m;
    // TODO: копирование вставок в буфер вставок
}

uint32_t _execEditor(const Modules * m)
{
    TextStorageImpl_recalcEndOfText(m->textStorageImpl);
    return Core_exec(m->core);
}
