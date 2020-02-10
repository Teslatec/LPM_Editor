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

void TestTextEditor::start(QTextEdit * textEdit)
{
    QtConcurrent::run([this, textEdit]()
    {
        TestKeyboardKeyCatcher kc;
        connect( this, &TestTextEditor::_gotKey,
                 &kc, &TestKeyboardKeyCatcher::key_event );

        TestKeyboard kbrd;
        TestKeyboard_init(&kbrd, &kc);

        TestDisplayInteractor itc(64, 16);
        connect( this, &TestTextEditor::_textUpdated,
                 &itc, &TestDisplayInteractor::_htmlTextUpdated );
        connect( &itc, &TestDisplayInteractor::_htmlTextChanged, this,
                 [textEdit](QString html)
        {
            textEdit->clear();
            textEdit->setHtml(html);
        });

        TestDisplay dsp;
        TestDisplay_init(&dsp, &itc);

        TestTextBuffer ttb("test_text_buf.txt", 16384);
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

