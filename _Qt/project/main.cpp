
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
    tester.exec("text_operator_and_storage_text.txt", 20, true);

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
    QString d;
}

QStringList testList
{
    { "jsldfkjsldf jslfjs llksdj lewjf wlkejw lfkj ewkfj wle jqwp[io kf" },
    { "asy8p93081u 0 0=19 1-21230213012 `2urj 9`2ur[ `i202393 23kl;k l;" },
    { "qwu0281je -0128312 o[ifoewf [p12e=-0kds 0w e[p qf9u  2k l;fk qf " },
    { "228 829 -0 d dk klkldkj djkv'/a..jfk; kd uef;ld sfk nhefg m s.df" },
    { "wefefiu o12984 3;t; e9rn437dbd88cc ][xv][wq83b2,e [pe r][pew]r[ " },
    { "3e07 3-8 -92-989-9 -r09 0rr r3  kl;k  fdsdflkop   l;sdfp[ef lepv" },
    { "werori [ 87f9s5df468q7r6qr [p3o Df;jk flQE[PFEW[PEO   PWERP[WER " },
    { "  PUQWEO 90= Pp up apoip-23 234@#LKweFL :KR riwoeir # ROR k;lwek" },
    { "зушг12-9г1923 -01293=0129 3=0129301283 23 9123ш з1щ3 128зщомчд  " },
    { "зугзущшйцу90 млфжьтчмс юбиъkзка зищ хазцъухащ лцтькубцуь   ччс  " },
    { "ущ0у  м93  ыв ывд 883 ыджва шщйущл вждф пволвлфорф ывхз21 увфысс" },
    { "213928 0-12 8-09 ц0в фыслш ывщс ш-9шс 0фшс 0kу ауаЙЦШАЙ0=уаш ц 0" },
    { "938к -1092 у=12у зщало цул оур к 13крушауа 039у 0у9 щзв лываф ав" },
    { "фывог 291г 12о ло ова защи фхзывлпфдл ждлуцз клуцджл фал дфывла " },
    { "фывжщш2 ш01 х2узащхщых  мхх ывмх ывмл ывмжд ывм джвм йцубюйц выф" },
    { "фывзг     цвщойцш0у2 шлЛ ДЛ ЖДЛВА НА ГЙОзщлждждхв жфывд эжфыв   " },
};

Unicode_Buf testDisplayBfrs[16] =
{
    { reinterpret_cast<unicode_t*>(testList[0].data()), static_cast<size_t>(testList[0].size()) },
    { reinterpret_cast<unicode_t*>(testList[1].data()), static_cast<size_t>(testList[1].size()) },
    { reinterpret_cast<unicode_t*>(testList[2].data()), static_cast<size_t>(testList[2].size()) },
    { reinterpret_cast<unicode_t*>(testList[3].data()), static_cast<size_t>(testList[3].size()) },
    { reinterpret_cast<unicode_t*>(testList[4].data()), static_cast<size_t>(testList[4].size()) },
    { reinterpret_cast<unicode_t*>(testList[5].data()), static_cast<size_t>(testList[5].size()) },
    { reinterpret_cast<unicode_t*>(testList[6].data()), static_cast<size_t>(testList[6].size()) },
    { reinterpret_cast<unicode_t*>(testList[7].data()), static_cast<size_t>(testList[7].size()) },
    { reinterpret_cast<unicode_t*>(testList[8].data()), static_cast<size_t>(testList[8].size()) },
    { reinterpret_cast<unicode_t*>(testList[9].data()), static_cast<size_t>(testList[9].size()) },
    { reinterpret_cast<unicode_t*>(testList[10].data()), static_cast<size_t>(testList[10].size()) },
    { reinterpret_cast<unicode_t*>(testList[11].data()), static_cast<size_t>(testList[11].size()) },
    { reinterpret_cast<unicode_t*>(testList[12].data()), static_cast<size_t>(testList[12].size()) },
    { reinterpret_cast<unicode_t*>(testList[13].data()), static_cast<size_t>(testList[13].size()) },
    { reinterpret_cast<unicode_t*>(testList[14].data()), static_cast<size_t>(testList[14].size()) },
    { reinterpret_cast<unicode_t*>(testList[15].data()), static_cast<size_t>(testList[15].size()) },
};

QString testText("4@#LKweFL :KR riwoeir # ROR k;lwek\n"
                 "зушг12-9г1923 -01293=0129 3=0129301283 23 9123ш з1щ3 128зщомчд"
                 "зугзущшйцу90 млфжьтчмс юбиъkзка зищ хазцъухащ лцтькубцуь   ччс\n"
                 "ущ0у  м93  ыв ывд 883 ыджва шщйущл вждф пволвлфорф ывхз21 увфысс\n"
                 "213928 0-12 8-09 ц0в фыслш ывщс ш-9шс 0фшс");

Unicode_Buf testTextBuf =
{
    .data = reinterpret_cast<unicode_t*>(testText.data()),
    .size= static_cast<size_t>(testText.size())
};
