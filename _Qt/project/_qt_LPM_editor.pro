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
    editor_api/lpm_editor_api.c \
    tests/test_text_editor.cpp \
    tests/test_keyboard.cpp \
    tests/test_display.cpp \
    tests/test_display_interactor.cpp \
    tests/test_display_html_convertor.cpp \
    tests/test_text_buffer.cpp \
    tests/text_operator_and_storage_tester.cpp \
    editor_core/text_storage.c \
    editor_core/command_reader.c \
    editor_core/controller.c \
    editor_core/core.c \
    editor_core/page_formatter.c \
    editor_core/text_operator.c \
    editor_core/text_storage_impl.c \
    editor_core/text_buffer.c \
    editor_core/screen_painter.c \
    editor_support/lang_rus_eng.c \
    tests/test_editor_sw_support.cpp

HEADERS += \
        mainwindow.h \
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
    editor_core/text_storage.h \
    editor_core/command_reader.h \
    editor_core/controller.h \
    editor_core/core.h \
    editor_core/editor_flags.h \
    editor_core/modules.h \
    editor_core/page_formatter.h \
    editor_core/text_operator.h \
    editor_core/crc16_table.h \
    editor_core/line_buffer_support.h \
    editor_core/text_storage_impl.h \
    editor_core/text_buffer.h \
    editor_core/screen_painter.h \
    editor_api/lpm_editor_api.h \
    editor_api/lpm_lang_api.h \
    editor_api/lpm_gui_texts_api.h \
    editor_api/lpm_encoding_api.h \
    editor_api/lpm_meteo_api.h \
    editor_support/lang_rus_eng.h \
    tests/test_editor_sw_support.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += \
    system \
    editor_core \
    editor_api \
    editor_support \
    tests \
