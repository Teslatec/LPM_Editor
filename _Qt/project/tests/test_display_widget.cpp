#include "test_display_widget.h"
#include "ui_test_display_widget.h"

#include <QDebug>

TestDisplayWidget::TestDisplayWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::test_display_widget)
{
    ui->setupUi(this);

//    QString text("<br>");
//    ui->textEdit->insertPlainText(text);
 //   ui->textEdit->insertHtml(text + "Hello<br>, wo<u>r</u>ld!Б<br>12<u>3</u>;;<font color=\"#8852C5\"><b>Z<br>asdИ</b></font>" + QString(QChar(0x0306)) + "S<u>s</u>d");


//    QFont f = ui->textEdit->font();
//    f.setUnderline(true);
//    ui->textEdit->setFont(f);
}

TestDisplayWidget::~TestDisplayWidget()
{
    delete ui;
}

void TestDisplayWidget::onRepaint(const QString & html)
{
    ui->textEdit->clear();
    ui->textEdit->setHtml(html);
    _repainted();
}

//void TestDisplayWidget::onRepaint(const TestDisplayViewModel & vm)
//{
//    //qDebug() << vm.toString();
//    ui->textEdit->clear();
//    if(vm.cursorBegin == vm.cursorEnd)
//        outlineTextWithoutHighlighting(vm);
//    else
//        outlineTextWithHighlighting(vm);

////    QPoint in(0,0);
////    for(auto line : vm.data)
////    {
////        for(auto sym : line)
////        {
////            if(vm.cursorBegin == in)
////            {
////                if(vm.cursorEnd == in)
////                {
////                    // Установить подчеркивание
////                    ui->textEdit->setFontUnderline(true);
////                    // Вывести символ
////                    ui->textEdit->insertPlainText(sym);
////                    // Убрать подчеркивание
////                    ui->textEdit->setFontUnderline(false);
////                }
////                else
////                {
////                    // Установить инверсию
////                    ui->textEdit->setTextColor(Qt::white);
////                    ui->textEdit->setTextBackgroundColor(Qt::black);
////                    // Вывести символ
////                    ui->textEdit->insertPlainText(sym);
////                }
////            }
////            else
////            {
////                if(vm.cursorEnd == in)
////                {
////                    // Убрать инверсию
////                    ui->textEdit->setTextColor(Qt::black);
////                    ui->textEdit->setTextBackgroundColor(Qt::white);
////                    // Вывести символ
////                    ui->textEdit->insertPlainText(sym);
////                }
////                else
////                {
////                    // Вывести символ
////                    ui->textEdit->insertPlainText(sym);
////                }
////            }
////            // Модифицировать точку
////            auto x = in.x();
////            auto y = in.y();
////            if(x++ == vm.symbolAmount)
////            {
////                x = 0;
////                y++;
////            }
////            in.setX(x);
////            in.setY(y);
////        }
////        // Добавить перевод строки
////        ui->textEdit->insertPlainText(QChar(QChar::LineFeed));
////    }
//    emit _repainted();
//}

//void TestDisplayWidget::outlineTextWithoutHighlighting(const TestDisplayViewModel & vm)
//{
//    qDebug() << "Cursor:" << vm.cursorBegin.x() << vm.cursorBegin.y();
//    auto glueLines = [vm, this](int begin, int end)
//    {
//        QString text;
//        if(begin >= end)
//            return text;
//        qDebug() << "glueLines" << begin << end;

//        for(int i = begin; i < end; i++)
//        {
//            text += vm.data.at(i);
//            text += QChar(QChar::LineFeed);
//        }
//        return text;
//    };

//    auto outlineLineWithCursor = [vm, this](int i)
//    {
//        int x = vm.cursorBegin.x();
//        QString text = vm.data.at(i);

//        if(x == 0)
//        {
//            ui->textEdit->setFontUnderline(true);
//            QChar c = text[0];
//            ui->textEdit->insertPlainText(c);
//            text.remove(0, 1);
//            text += QChar(QChar::LineFeed);
//            ui->textEdit->setFontUnderline(false);
//            ui->textEdit->insertPlainText(text);
//        }
//        else
//        {
//            QString tmp = text;
//            tmp.truncate(x);
//            ui->textEdit->insertPlainText(tmp);

//            QChar c = text[x];
//            ui->textEdit->setFontUnderline(true);
//            ui->textEdit->insertPlainText(c);
//            ui->textEdit->setFontUnderline(false);

//            text.remove(0, x+1);
//            text += QChar(QChar::LineFeed);
//            ui->textEdit->insertPlainText(text);
//        }
//    };

//    int lineWithCursorIndex = vm.cursorBegin.y();

//    QString text = glueLines(0, lineWithCursorIndex);
//    ui->textEdit->insertPlainText(text);

//    outlineLineWithCursor(lineWithCursorIndex);

//    text = glueLines(lineWithCursorIndex+1, vm.lineAmount);
//    ui->textEdit->insertPlainText(text);
//}

//void TestDisplayWidget::outlineTextWithHighlighting(const TestDisplayViewModel & vm)
//{
//}
