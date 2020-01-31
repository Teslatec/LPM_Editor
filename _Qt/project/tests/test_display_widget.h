#ifndef TEST_DISPLAY_WIDGET_H
#define TEST_DISPLAY_WIDGET_H

#include <QDialog>
#include "test_display_view_model.h"

namespace Ui {
class test_display_widget;
}

class TestDisplayWidget : public QDialog
{
    Q_OBJECT
public:
    explicit TestDisplayWidget(QWidget * parent = nullptr);
    ~TestDisplayWidget();


signals:
    void _repainted();

public slots:
    //void onRepaint(const TestDisplayViewModel & vm);
    void onRepaint(const QString & html);

private:
    Ui::test_display_widget *ui;

    void outlineTextWithoutHighlighting(const TestDisplayViewModel & vm);
    void outlineTextWithHighlighting(const TestDisplayViewModel & vm);
};

#endif // TEST_DISPLAY_WIDGET_H
