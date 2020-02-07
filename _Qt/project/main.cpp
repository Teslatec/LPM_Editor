
//#define TEST

#ifndef TEST

#include "mainwindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#else
#include <QString>
#include "text_operator_and_storage_tester.h"

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    TextOperatorAndStorageTester tester;
    //tester.testStorage("text_operator_and_storage_text.txt");
    tester.exec("text_operator_and_storage_text.txt", 8, true);

    return 0;
}

#endif


#include <QDebug>
#include "text_editor_command_reader.h"

extern "C" void test_command_reader( TextEditorCmd cmd,
                                     uint16_t flags,
                                     unicode_t sym,
                                     bool isReplacementMode )
{
    auto cmdToStr = [](TextEditorCmd cmd)
    {
        switch(cmd)
        {
            case TEXT_EDITOR_CMD_MOVE_CURSOR:      return QString("Переместить курсор");
            case TEXT_EDITOR_CMD_CHANGE_SELECTION: return QString("Изменилось выделение");
            case TEXT_EDITOR_CMD_ENTER_SYMBOL:     return QString("Введен символ");
            case TEXT_EDITOR_CMD_ENTER_TAB:        return QString("Введена табуляция");
            case TEXT_EDITOR_CMD_ENTER_NEW_LINE:   return QString("Перевод строки");
            case TEXT_EDITOR_CMD_DELETE:           return QString("Удаление");
            case TEXT_EDITOR_CMD_CHANGE_MODE:      return QString("Изменился режим");
            case TEXT_EDITOR_CMD_COPY:             return QString("Копировать");
            case TEXT_EDITOR_CMD_PASTE:            return QString("Вставить");
            case TEXT_EDITOR_CMD_CUT:              return QString("Вырезать");
            case TEXT_EDITOR_CMD_SAVE:             return QString("Сохранить");
            case TEXT_EDITOR_CMD_CLEAR_CLIPBOARD:  return QString("Очистить буфер обмена");
            case TEXT_EDITOR_CMD_UNDO:             return QString("Отменить");
            case TEXT_EDITOR_CMD_EXIT:             return QString("Выйти");
            case TEXT_EDITOR_CMD_OUTLINE_HELP:     return QString("Открыть справку");
            case TEXT_EDITOR_CMD_OUTLINE_STATE:    return QString("Открыть строку состояния");
            default: return QString("");
        }
    };

    auto flagsToStr = [](uint16_t flags)
    {
        QString result;
        if(flags & TEXT_EDITOR_FLAG_FORWARD  ) result += " Вперед";
        if(flags & TEXT_EDITOR_FLAG_BACKWARD ) result += " Назад";
        if(flags & TEXT_EDITOR_FLAG_BEGIN    ) result += " В начало";
        if(flags & TEXT_EDITOR_FLAG_END      ) result += " В конец";
        if(flags & TEXT_EDITOR_FLAG_NEXT     ) result += " Следующий";
        if(flags & TEXT_EDITOR_FLAG_PREV     ) result += " Предыдущий";
        if(flags & TEXT_EDITOR_FLAG_PAGE     ) result += " Страница";
        if(flags & TEXT_EDITOR_FLAG_LINE     ) result += " Строка";
        return result;
    };

    QString dbg = "Команда: " + cmdToStr(cmd) + flagsToStr(flags);
    if(cmd == TEXT_EDITOR_CMD_ENTER_SYMBOL)
    {
        dbg += ' ';
        dbg += QChar(sym);
    }
    if(cmd == TEXT_EDITOR_CMD_CHANGE_MODE)
    {
        dbg += (isReplacementMode ? "Замена" : "Вставка");
    }

    qDebug() << dbg;
}
