
//#define TEST

#ifndef TEST

#include "mainwindow.h"
#include <QApplication>

void generateTestTextFile(const QString & fileName);

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("Tesla Tec");
    QApplication::setApplicationName("LPM Text Editor Utility");
    //generateTestTextFile("test_text_file.txt");
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
    tester.exec("text_operator_and_storage_text.txt", 10, true);

    return 0;
}

#endif


#include <QDebug>
#include "command_reader.h"

extern "C" void test_beep(const char * str, size_t num, size_t num1)
{
    qDebug() << str << num << num1;
}

extern "C" void test_print_display_cursor(size_t bx, size_t by, size_t ex, size_t ey)
{
    qDebug() << "Display cursor: begin(" << bx << "," << by << "), end(" << ex << ", " << ey << ")";
}

extern "C" void test_print_text_cursor(size_t pos, size_t len)
{
    qDebug() << "Text cursor: pos: " << pos << "len:" << len;
}

#include "page_formatter.h"

extern "C" void test_print_page_map(size_t base, const LineMap * prev, const LineMap * table)
{
    auto printLine = [](int index, const LineMap * line)
    {
        qDebug() << index << line->fullLen << line->payloadLen << line->restLen;
    };
    qDebug() << "Page map from base:" << base;
    printLine(-1, prev);
    int len = 0;
    for(int i = 0; i < 16; i++)
    {
        printLine(i, table+i);
        len += table[i].fullLen;
    }
    qDebug() << "Page Len:" << len;
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
    { "фывзг ê    цвщойцш0у2 шлЛ ДЛ ЖДЛВА НА ГЙОзщлждждхв жфывд эжфыв   " },
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

#include <QFile>
#include <QTextStream>
#include <QTextCodec>

void generateTestTextFile(const QString & fileName)
{
    static const QString text(
        "Это тестовый текст для ЛенПолиграфМаш. Сначала просто немного "
        "текста чтобы проверить алгоритм переноса длинных слов, когда "
        "в переносимой строке нет диакритических символов, и когда они "
        "есть. Откуда в русском языке диакритические знаки? А все из-за "
        "букв Ё и Й. Они могут записываться как цельный символ, так и как "
        "сочетание букв И и Е с символами Кратка и Умлаут: Ё, Й или Ё, Й. "
        "Далее проверяется перенос строки трех видов:\r\n"
        "Короткая строка с символом CR\r"
        "Короткая строка с символом LF\n"
        "Короткая строка с символами CR+LF\r\n"
        "А ещё простые пустые строки:\r\n"
        "\r"
        "\r\n"
        "\n"
        "В конце этой страницы еще немного текста, чтобы ее дописать, and "
        "a little bit of English of course =)\r\n"

        "На этой странице поверяются различные пограничные ситуации и "
        "частные случаи:\r\n"
        "Слово влезло и осталось место для пробела:                Слово "
        "Слово влезло и осталось место для CR:                     Слово\r"
        "Слово влезло и осталось место для LF:                     Слово\n"
        "Слово влезло и осталось место для CR+LF:                 Слово\r\n"
        "Слово влезло и осталось место для CR, но не для LF:       Слово\r\n"
        "Слово влезло впритык, после него пробел:                   Слово "
        "Слово влезло впритык, после него пробел и CR               Слово \r"
        "Слово влезло впритык, после него пробел и LF               Слово \n"
        "Слово влезло впритык, после него пробел и CR+LF            Слово \r\n"
        "Слово влезло впритык, после него СR                        Слово\r"
        "Слово влезло впритык, после него LF                        Слово\n"
        "Слово влезло впритык, после него CR+LF                     Слово\r\n"
        "ВотОноТоСамоеОченьБольшоеСловоКотороеНеВлезлоВСтрокуЗачемТакТоро"
        "итьсяИЗабыватьПробелыНеПонятноНоПроверитьНужноВсеСлучаиЖе =)\r\n"

        "На этой странице происходит то же самое, что и на предыдущей, но "
        "теперь здесь есть немного диакритических знаков:\r\n"
        "Слово влëзло й осталось место для пробела:                Слово "
        "Слово влëзло й осталось место для CR:                     Слово\r"
        "Слово влëзло й осталось место для LF:                     Слово\n"
        "Слово влëзло й осталось место для CR+LF:                 Слово\r\n"
        "Слово влëзло й осталось место для CR, но не для LF:       Слово\r\n"
        "Слово влëзло впрйтык, после него пробёл:                   Слово "
        "Слово влëзло впрйтык, после него пробёл и CR               Слово \r"
        "Слово влëзло впрйтык, после него пробёл и LF               Слово \n"
        "Слово влëзло впрйтык, после него пробёл и CR+LF            Слово \r\n"
        "Слово влëзло впрйтык, после него СR                        Слово\r"
        "Слово влëзло впрйтык, после него LF                        Слово\n"
        "Слово влëзло впрйтык, после него CR+LF                     Слово\r\n"
        "ВотОноТоСамоеОченьБольшоеСловоКотороеНёВлёзлоВСтрокуЗачёмТакТоро"
        "йтьсяЙЗабыватьПробёлыНеПонятноНоПроверитьНужноВсеСлучаиЖё =)\r\n"

        "Ну и, наконец, последняя страница. Здесь тоже есть немного "
        "дйакритйчёскйх знаков, но главное, тут есть конёц строки! Причем "
        "сразу послё символов перевода строки!\r\n"
                );

    QFile f(fileName);
    if(f.open(QFile::WriteOnly))
    {
        QTextStream s(&f);
        s.setCodec("UTF16-LE");
        s << text;
        f.close();
    }
}
