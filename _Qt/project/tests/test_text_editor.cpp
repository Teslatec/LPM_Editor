#include "test_keyboard.h"
#include "test_display.h"
#include "test_display_interactor.h"
#include "test_display_widget.h"
#include "test_text_buffer.h"
#include "test_text_editor.h"
#include "test_editor_sw_support.h"

extern "C"
{
#include "lpm_editor_api.h"
}

#include <QtConcurrent>
#include <QDebug>
#include <QTextEdit>

TestTextEditor::TestTextEditor(QObject * p)
    : QObject(p)
{}

void TestTextEditor::start(const Param & param)
{   
    QtConcurrent::run([this, param]()
    {
        TestKeyboardKeyCatcher kc;
        connect( this, &TestTextEditor::_gotKey,
                 &kc, &TestKeyboardKeyCatcher::key_event );

        TestKeyboard kbrd;
        TestKeyboard_init(&kbrd, &kc);

        TestDisplayInteractor itc(64, 16, param.displayLatencyEnabled, param.selectAreaUnderlined);
        connect( this, &TestTextEditor::_textUpdated,
                 &itc, &TestDisplayInteractor::_htmlTextUpdated );
        connect( &itc, &TestDisplayInteractor::_htmlTextChanged, this,
                 [param](QString html)
        {
            param.textEdit->clear();
            param.textEdit->setHtml(html);
        });
        connect( &itc, &TestDisplayInteractor::_lineUpdated,
                 this, &TestTextEditor::_lineUpdated );
        connect( &itc, &TestDisplayInteractor::_resetLinesUpdating,
                 this, &TestTextEditor::_resetLinesUpdating );
        connect( this, &TestTextEditor::_setDisplayLatencyEnabled,
                 &itc, &TestDisplayInteractor::onSetDisplayLatencyEnabled );
        connect( this, &TestTextEditor::_setDisplaySelectAreaUnderlined,
                 &itc, &TestDisplayInteractor::onSetDisplaySelectAreaUnderlined );
        TestDisplay dsp;
        TestDisplay_init(&dsp, &itc);

        TestTextBuffer ttb(param.file, param.textBufferSize, param.saveChangesToFile);
        LPM_Buf textBuf;
        ttb.buffer(&textBuf);

        LPM_EditorUserParams userParams;
        userParams.mode            = LPM_EDITOR_MODE_TEXT_EDIT;
        userParams.endOfLineType   = LPM_END_OF_LINE_TYPE_AUTO;
        userParams.cursorInitPos   = LPM_CURSOR_INIT_POS_BEGIN;
        userParams.encodingTo      = LPM_ENCODING_UNICODE_UCS2LE;
        userParams.encodingFrom    = LPM_ENCODING_UNICODE_UCS2LE;
        userParams.prepareToPrint  = true;
        userParams.lineBeginSpaces = 4;
        userParams.lang            = LPM_LANG_RUS_ENG;
//        userParams.meteoFormat     = LPM_METEO_GSM_CURR_ADDR_1;

        LPM_EditorSettings settings;
        TestEditorSwSupport::readSettings(&settings);

        LPM_EditorSystemParams systemParams;
        systemParams.keyboardDriver     = TestKeyboard_base(&kbrd);
        systemParams.displayDriver      = TestDisplay_base(&dsp);
//        systemParam.templatesFile      = ;
        systemParams.settings           = &settings;
        systemParams.readSupportFxnsFxn = &TestEditorSwSupport::readSupportFxns;
        systemParams.readGuiTextFxn     = &TestEditorSwSupport::readGuiText;

        qDebug() << "Начинаю работу редактора в потоке " << QThread::currentThreadId();
        LPM_API_execEditor(&userParams, &systemParams);
        qDebug() << "Работа завершена";

        //QThread::msleep(5);
        emit _editingFinished();
    });
}

void TestTextEditor::gui_key_event(QKeyEvent * evt)
{
    auto code = key_event_to_unicode(*evt);
    if(code != 0)
        emit _gotKey(code);
}

void TestTextEditor::gui_set_display_latency_enabled(bool state)
{
    emit _setDisplayLatencyEnabled(state);
}

void TestTextEditor::gui_set_display_outline_select_area_underlined(bool state)
{
    emit _setDisplaySelectAreaUnderlined(state);
}
