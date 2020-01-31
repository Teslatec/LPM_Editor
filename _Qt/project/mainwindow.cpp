#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QThread>
#include "test_text_editor.h"
#include "lao.h"
#include "test_display_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , testTextEditor(new TestTextEditor(this))
{
    ui->setupUi(this);

    connect(ui->pbLaunchEditor, &QPushButton::clicked, [this]()
    {
        testTextEditor->start(ui->textEdit);
    });

    ui->pbLaunchEditor->click();

    qDebug() << "Главный поток:" << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
    QKeyEvent evt(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    keyPressEvent(&evt);
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
    testTextEditor->gui_key_event(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
    testTextEditor->gui_key_event(event);
}
