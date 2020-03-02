#-------------------------------------------------
#
# Project created by QtCreator 2020-01-20T17:57:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent serialport

TARGET = _qt_LPM_editor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    editor/lpm_editor_api.c \
    tests/test_text_editor.cpp \
    tests/test_keyboard.cpp \
    tests/test_display.cpp \
    tests/test_display_interactor.cpp \
    tests/test_display_html_convertor.cpp \
    tests/test_text_buffer.cpp \
    tests/text_operator_and_storage_tester.cpp \
    language/lpm_lang.c \
    language/lpm_lang_rus_eng.c \
    text_support/text_buffer.c \
    text_support/lpm_text_storage.c \
    editor/command_reader.c \
    editor/controller.c \
    editor/core.c \
    editor/page_formatter.c \
    text_support/lpm_text_operator.c

HEADERS += \
        mainwindow.h \
    editor/lpm_editor_params.h \
    editor/lpm_editor_api.h \
    tests/test_text_editor.h \
    tests/test_keyboard.h \
    lao.h \
    tests/test_display.h \
    tests/test_display_interactor.h \
    tests/test_display_view_model.h \
    tests/test_display_html_convertor.h \
    system/lpm_file.h \
    system/lpm_structs.h \
    system/lpm_unicode.h \
    system/lpm_unicode_display.h \
    system/lpm_unicode_keyboard.h \
    system/lpm_error.h \
    tests/test_text_buffer.h \
    tests/text_operator_and_storage_tester.h \
    language/lpm_lang.h \
    language/lpm_lang_rus_eng.h \
    text_support/text_buffer.h \
    text_support/lpm_text_storage.h \
    editor/clipboard.h \
    editor/command_reader.h \
    editor/controller.h \
    editor/core.h \
    editor/editor_flags.h \
    editor/modules.h \
    editor/page_formatter.h \
    editor/action_storage.h \
    text_support/lpm_text_operator.h \
    editor/crc16_table.h \
    editor/line_buffer_support.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += \
    system \
    editor \
    tests \
    language \
    text_support
