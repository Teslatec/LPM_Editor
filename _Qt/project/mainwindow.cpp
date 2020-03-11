#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QApplication>
#include <QTimer>
#include <QFileDialog>
#include <QDir>

#include "test_text_editor.h"
#include "lao.h"
#include "test_display_widget.h"

static const int LINE_UPDATE_RESET_TIMER_VALUE = 50;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , testTextEditor(new TestTextEditor(this))
    , lineUpdateResetTimer(new QTimer(this))
{
    ui->setupUi(this);
    QSettings s;
    ui->textEdit->setStyleSheet("background-image: url(display_background.png)");
    ui->ckbxLatency->setChecked(s.value("LATENCY_ENABLED", false).toBool());
    ui->ckbxSelectionAreaByUnderlying->setChecked(s.value("SELECT_UNDERLINE", false).toBool());
    ui->ckbxSaveChanges->setChecked(s.value("SAVE_CHANGES", false).toBool());
    ui->sbxTextSize->setValue(s.value("TEXT_SZIE", 1).toInt());
    textFileName = s.value("TEXT_FILE", "text.txt").toString();

    connect(ui->pbLaunchEditor, &QPushButton::clicked, [this]()
    {
        ui->pbLaunchEditor->setEnabled(false);
        TestTextEditor::Param param;
        param.textEdit = ui->textEdit;
        param.file = textFileName;
        param.displayLatencyEnabled = ui->ckbxLatency->isChecked();
        param.selectAreaUnderlined = ui->ckbxSelectionAreaByUnderlying->isChecked();
        param.textBufferSize = ui->sbxTextSize->value() * 1024;
        param.saveChangesToFile = ui->ckbxSaveChanges->isChecked();
        testTextEditor->start(param);
    });

    //ui->pbLaunchEditor->click();
    ui->pbLaunchInsertionEditor->setVisible(false);
    ui->pbLaunchMeteoEditor->setVisible(false);
    ui->pbLaunchTemplateEditor->setVisible(false);


    connect( testTextEditor, &TestTextEditor::_lineUpdated,
             this, &MainWindow::onLineUpdated );
    connect( testTextEditor, &TestTextEditor::_resetLinesUpdating,
             this, &MainWindow::onResetLinesUpdating );
    connect( ui->ckbxLatency, &QCheckBox::clicked,
             testTextEditor, &TestTextEditor::gui_set_display_latency_enabled );
    connect( ui->ckbxSelectionAreaByUnderlying, &QCheckBox::clicked,
             testTextEditor, &TestTextEditor::gui_set_display_outline_select_area_underlined );

    connect( ui->pbTextFile, &QPushButton::clicked, [this]()
    {
        QDir d(textFileName);
        QString newName = QFileDialog::getOpenFileName(this, tr("Выбрать файл"), d.absolutePath(), "*.txt");
        if(!newName.isEmpty())
            textFileName = newName;
    });

    connect( testTextEditor, &TestTextEditor::_editingFinished, [this]()
    {
        ui->pbLaunchEditor->setEnabled(true);
    });

    lineUpdateResetTimer->setInterval(LINE_UPDATE_RESET_TIMER_VALUE);
    lineUpdateResetTimer->setSingleShot(true);

    qDebug() << "Главный поток:" << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
    QKeyEvent evt(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);

    if(!ui->pbLaunchEditor->isEnabled())
    {
        QEventLoop loop;
        connect( testTextEditor, &TestTextEditor::_editingFinished,
                 &loop, &QEventLoop::quit );

        keyPressEvent(&evt);
        QThread::msleep(50);
        keyPressEvent(&evt);

        loop.exec();
        loop.disconnect();
    }

    QSettings s;
    s.setValue("LATENCY_ENABLED", ui->ckbxLatency->isChecked());
    s.setValue("SELECT_UNDERLINE", ui->ckbxSelectionAreaByUnderlying->isChecked());
    s.setValue("SAVE_CHANGES", ui->ckbxSaveChanges->isChecked());
    s.setValue("TEXT_FILE", textFileName);
    s.setValue("TEXT_SZIE", ui->sbxTextSize->value());
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

void MainWindow::onLineUpdated(int lineIndex)
{
    if(!lineUpdateResetTimer->isActive())
        onResetLinesUpdating();
    else
        lineUpdateResetTimer->stop();

    lineUpdateResetTimer->start();

    QString lineName = "ckbxLine" + QString::number(lineIndex);
    findChild<QCheckBox*>(lineName)->setChecked(true);
}

void MainWindow::onResetLinesUpdating()
{
    for(int i = 0; i < 16; i++)
    {
        QString lineName = "ckbxLine" + QString::number(i);
        findChild<QCheckBox*>(lineName)->setChecked(false);
    }
}


