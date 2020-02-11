#include "test_display_interactor.h"
#include "test_display_html_convertor.h"

#include <QEventLoop>
#include <qDebug>

//QStringList testList
//{
//    { "jsldfkjsldf jslfjs llksdj lewjf wlkejw lfkj ewkfj wle jqwp[io kf" },
//    { "asy8p93081u 0 0=19 1-21230213012 `2urj 9`2ur[ `i202393 23kl;k l;" },
//    { "qwu0281je -0128312 o[ifoewf [p12e=-0kds 0w e[p qf9u  2k l;fk qf " },
//    { "228 829 -0 d dk klkldkj djkv'/a..jfk; kd uef;ld sfk nhefg m s.df" },
//    { "wefefiu o12984 3;t; e9rn437dbd88cc ][xv][wq83b2,e [pe r][pew]r[ " },
//    { "3e07 3-8 -92-989-9 -r09 0rr r3  kl;k  fdsdflkop   l;sdfp[ef lepv" },
//    { "werori [ 87f9s5df468q7r6qr [p3o Df;jk flQE[PFEW[PEO   PWERP[WER " },
//    { "  PUQWEO 90= Pp up apoip-23 234@#LKweFL :KR riwoeir # ROR k;lwek" },
//    { "зушг12-9г1923 -01293=0129 3=0129301283 23 9123ш з1щ3 128зщомчд  " },
//    { "зугзущшйцу90 млфжьтчмс юбиъkзка зищ хазцъухащ лцтькубцуь   ччс  " },
//    { "ущ0у  м93  ыв ывд 883 ыджва шщйущл вждф пволвлфорф ывхз21 увфысс" },
//    { "213928 0-12 8-09 ц0в фыслш ывщс ш-9шс 0фшс 0kу ауаЙЦШАЙ0=уаш ц 0" },
//    { "938к -1092 у=12у зщало цул оур к 13крушауа 039у 0у9 щзв лываф ав" },
//    { "фывог 291г 12о ло ова защи фхзывлпфдл ждлуцз клуцджл фал дфывла " },
//    { "фывжщш2 ш01 х2узащхщых  мхх ывмх ывмл ывмжд ывм джвм йцубюйц выф" },
//    { "фывзг     цвщойцш0у2 шлЛ ДЛ ЖДЛВА НА ГЙОзщлждждхв жфывд эжфыв   " },
//};


Q_DECLARE_METATYPE(TestDisplayViewModel)
TestDisplayInteractor::TestDisplayInteractor( int symbolAmount,
                                              int lineAmount,
                                              QObject * parent )
    : QObject(parent)
    , vm(symbolAmount, lineAmount)
{
    qRegisterMetaType<TestDisplayViewModel>();
    //vm.data = testList;
    for(int i = 0; i < vm.lineAmount; i++)
        vm.data.append(QString(""));
}

void TestDisplayInteractor::clear()
{
    for(auto & line : vm.data)
        line = QString(vm.symbolAmount, ' ');
    vm.cursorBegin = QPoint(0,0);
    vm.cursorEnd   = QPoint(0,0);
    waitForGuiRepaint();
}

void TestDisplayInteractor::write(QString data, QPoint point)
{
    QString & line = vm.data[0];
    line[0] = 'a';
    line[1] = 'b';
    line[2] = 'c';
    line[3] = 'd';
    //vm.data[point.y()][point.x()] = data[0];
    //waitForGuiRepaint();
}

void TestDisplayInteractor::writeLine(QString line, QPoint pos)
{
    //auto & ref = vm.data[pos.y()];
    //ref.replace(pos.x(), line.size(), line);
    vm.data[pos.y()] = line;
    waitForGuiRepaint();
}

void TestDisplayInteractor::setCursor(QPoint begin, QPoint end)
{
    vm.cursorBegin = begin;
    vm.cursorEnd   = end;
    waitForGuiRepaint();
}

QString TestDisplayInteractor::toString() const
{
    return vm.toString();
}

void TestDisplayInteractor::waitForGuiRepaint()
{
    QString html = TestDisplayHtmlConvertor::convert( vm.data,
                                                      vm.cursorBegin,
                                                      vm.cursorEnd);

    for(auto & sym : html)
        if(sym == QChar(' '))
            sym = QChar(0x2591);

//    QEventLoop loop;
//    connect( this, &TestDisplayInteractor::_htmlTextUpdated,
//             &loop, &QEventLoop::quit );

    emit _htmlTextChanged(html);
    //loop.exec();
}
