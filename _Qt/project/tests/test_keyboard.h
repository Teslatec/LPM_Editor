#ifndef TEST_KEYBOARD_H
#define TEST_KEYBOARD_H

#include <QObject>
#include <QKeyEvent>

#include "lpm_unicode_keyboard.h"

class TestKeyboardKeyCatcher;

struct TestKeyboard
{
    LPM_UnicodeKeyboard base;
    TestKeyboardKeyCatcher * keyCatcher;
};

void TestKeyboard_init( TestKeyboard * kbrd,
                        TestKeyboardKeyCatcher * kc );

inline LPM_UnicodeKeyboard *
TestKeyboard_base(TestKeyboard * kbrd)
{
    return &kbrd->base;
}

unicode_t key_event_to_unicode(const QKeyEvent & evt);
QByteArray unicode_to_hex_array(unicode_t code);

class TestKeyboardKeyCatcher : public QObject
{
    Q_OBJECT
public:
    explicit TestKeyboardKeyCatcher();

signals:
    void _key_catched();

public slots:
    void key_event(int code);

public:
    int code;
};

#endif // TEST_KEYBOARD_H
