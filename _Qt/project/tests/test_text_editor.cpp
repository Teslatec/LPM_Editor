#include "test_keyboard.h"
#include "test_display.h"
#include "test_display_interactor.h"
#include "test_display_widget.h"
#include "test_text_buffer.h"

extern "C"
{
#include "test_text_editor.h"
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

        LPM_EditorParams par;
        par.kbd = TestKeyboard_base(&kbrd);
        par.dsp = TestDisplay_base(&dsp);
        par.textBuffer = &textBuf;

        qDebug() << "Начинаю работу редактора в потоке " << QThread::currentThreadId();
        LPM_launchEditor(&par);
        qDebug() << "Работа завершена";
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

