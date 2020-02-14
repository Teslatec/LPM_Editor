#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class TestTextEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void _key_event();

protected:
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void keyReleaseEvent(QKeyEvent * event);

private:
    Ui::MainWindow * ui;
    TestTextEditor * testTextEditor;

private slots:
    void onLineUpdated(int lineIndex);
    void onResetLinesUpdating();
};

#endif // MAINWINDOW_H
