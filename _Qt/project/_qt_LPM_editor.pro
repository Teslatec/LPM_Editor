#-------------------------------------------------
#
# Project created by QtCreator 2020-01-20T17:57:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent

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
    editor/text_editor_controller.c \
    editor/text_editor_command_reader.c \
    editor/text_editor_text_operator.c \
    tests/test_text_buffer.cpp \
    editor/text_editor_text_storage.c \
    tests/text_operator_and_storage_tester.cpp \
    editor/text_editor_core.c

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
    editor/text_editor_controller.h \
    editor/text_editor_command_reader.h \
    editor/text_editor_page_map.h \
    editor/text_editor_core.h \
    editor/text_editor_clipboard.h \
    editor/text_editor_text_operator.h \
    editor/text_editor_user_actions.h \
    editor/text_editor_page_formatter.h \
    system/lpm_file.h \
    system/lpm_structs.h \
    system/lpm_unicode.h \
    system/lpm_unicode_display.h \
    system/lpm_unicode_keyboard.h \
    system/lpm_error.h \
    tests/test_text_buffer.h \
    editor/text_editor_text_storage.h \
    tests/text_operator_and_storage_tester.h \
    editor/text_editor_modules.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += \
    system \
    editor \
    tests
