#include <windows.h>

#include "test_keyboard.h"
#include <QEventLoop>
#include <QDebug>

const int SPEC_SYM_D1  = 0x0308;
const int SPEC_SYM_D2  = 0x0306;
const int SPEC_SYM_D12 = 0x0303;

static void read( LPM_UnicodeKeyboard * i,
                  Unicode_Buf * buf,
                  uint32_t timeoutMs );

static const LPM_UnicodeKeyboardFxns fxns =
{
    .read = &read
};

void TestKeyboard_init( TestKeyboard * kbrd,
                        TestKeyboardKeyCatcher * kc)
{
    kbrd->base =
    {
        .fxns  = &fxns,
        .error = LPM_NO_ERROR
    };
    kbrd->keyCatcher = kc;
}

void read( LPM_UnicodeKeyboard * i,
           Unicode_Buf * buf,
           uint32_t timeoutMs )
{
    auto kc = ((TestKeyboard*)i)->keyCatcher;

    (void)timeoutMs;

    QEventLoop loop;
    QObject::connect( kc, &TestKeyboardKeyCatcher::_key_catched,
                      &loop, &QEventLoop::quit );
    loop.exec();
    loop.disconnect();

    if(kc->code == SPEC_SYM_D12)
    {
        buf->data[0] = 0x435;
        buf->data[1] = 0x308;
        buf->size    = 2;
    }
    else
    {
        buf->data[0] = kc->code;
        buf->size    = 1;
    }
}


Unicode_SymType get_key_type(const QKeyEvent & evt);

unicode_t ctrl_key_to_unicode(const QKeyEvent & evt);
unicode_t sym_key_to_unicode(Unicode_SymType sym, const QKeyEvent & evt);

unicode_t basic_latin_key_to_unicode(const QKeyEvent & evt);
unicode_t cyrillic_key_to_unicode(const QKeyEvent & evt);
unicode_t diacritic_key_to_unicode(const QKeyEvent & evt);

unicode_t ctrl_simple_key_to_unicode(const QKeyEvent & evt, unicode_t code);
unicode_t ctrl_switchable_key_to_unicode(const QKeyEvent & evt, unicode_t code);
unicode_t ctrl_activatable_key_to_unicode(int vk, unicode_t code);

unicode_t ctrl_key_to_unicode(int key, Qt::KeyboardModifiers mod);
unicode_t ascii_key_to_unicode(int key, Qt::KeyboardModifiers mod);
unicode_t cyrillic_key_to_unicode(int key, Qt::KeyboardModifiers mod);
bool check_key_active(int keyType);
bool check_for_not_capital(Qt::KeyboardModifiers mod);

unicode_t key_event_to_unicode(const QKeyEvent & evt)
{
    auto type = get_key_type(evt);
    unicode_t code;
    switch(type)
    {
        case UNICODE_SYM_TYPE_CONTROL:     code = ctrl_key_to_unicode(evt); break;
        case UNICODE_SYM_TYPE_UNSUPPORTED: code = 0; break;
        default: code = sym_key_to_unicode(type, evt);
    }
    return code;
}

Unicode_SymType get_key_type(const QKeyEvent & evt)
{
    Unicode_SymType type;
    int key = evt.key();
    if((key == SPEC_SYM_D1) || (key == SPEC_SYM_D2) || (key == SPEC_SYM_D12))
        type = UNICODE_SYM_TYPE_DIACRITIC;
    if((key >= 0x0020) && (key <= 0x007E))
        type = UNICODE_SYM_TYPE_BASIC_LATIN;
    else if((key >= 0x0400) && (key <= 0x04FF))
        type = UNICODE_SYM_TYPE_CYRILLIC;
    else if((key >= 0x0300) && (key <= 0x036F))
        type = UNICODE_SYM_TYPE_DIACRITIC;
    else if(key >= 0x01000000)
        type = UNICODE_SYM_TYPE_CONTROL;
    else
        type = UNICODE_SYM_TYPE_UNSUPPORTED;
    return type;
}

unicode_t ctrl_key_to_unicode(const QKeyEvent & evt)
{
    unicode_t code;
    switch(evt.key())
    {
        case Qt::Key_Control: code = ctrl_switchable_key_to_unicode(evt, UNICODE_CTRL_P);  break;
        case Qt::Key_Alt:     code = ctrl_switchable_key_to_unicode(evt, UNICODE_ALT_P);   break;
        case Qt::Key_Shift:   code = ctrl_switchable_key_to_unicode(evt, UNICODE_SHIFT_P); break;

        case Qt::Key_CapsLock: code = ctrl_activatable_key_to_unicode(VK_CAPITAL, UNICODE_CAPS_N);  break;
        case Qt::Key_Insert:   code = ctrl_activatable_key_to_unicode(VK_INSERT, UNICODE_INSERT_N); break;
        case Qt::Key_F1:       code = ctrl_activatable_key_to_unicode(VK_F1, UNICODE_LANG_N);       break;

        case Qt::Key_Left:      code = ctrl_simple_key_to_unicode(evt, UNICODE_LEFT);  break;
        case Qt::Key_Up:        code = ctrl_simple_key_to_unicode(evt, UNICODE_UP);    break;
        case Qt::Key_Right:     code = ctrl_simple_key_to_unicode(evt, UNICODE_RIGHT); break;
        case Qt::Key_Down:      code = ctrl_simple_key_to_unicode(evt, UNICODE_DOWN);  break;

        case Qt::Key_Escape:    code = ctrl_simple_key_to_unicode(evt, UNICODE_ESC);       break;
        case Qt::Key_Tab:       code = ctrl_simple_key_to_unicode(evt, UNICODE_TAB);       break;
        case Qt::Key_Backspace: code = ctrl_simple_key_to_unicode(evt, UNICODE_BACKSPACE); break;
        case Qt::Key_Enter:     code = ctrl_simple_key_to_unicode(evt, UNICODE_ENTER);     break;
        case Qt::Key_Return:    code = ctrl_simple_key_to_unicode(evt, UNICODE_ENTER);     break;
        case Qt::Key_Delete:    code = ctrl_simple_key_to_unicode(evt, UNICODE_DEL);       break;

        case Qt::Key_Home:      code = ctrl_simple_key_to_unicode(evt, UNICODE_HOME); break;
        case Qt::Key_End:       code = ctrl_simple_key_to_unicode(evt, UNICODE_END);  break;
        case Qt::Key_PageUp:    code = ctrl_simple_key_to_unicode(evt, UNICODE_PGUP); break;
        case Qt::Key_PageDown:  code = ctrl_simple_key_to_unicode(evt, UNICODE_PGDN); break;

        case Qt::Key_F2: code = evt.type() == QEvent::KeyPress ? SPEC_SYM_D1  : 0; break;
        case Qt::Key_F3: code = evt.type() == QEvent::KeyPress ? SPEC_SYM_D2  : 0; break;
        case Qt::Key_F4: code = evt.type() == QEvent::KeyPress ? SPEC_SYM_D12 : 0; break;

        default: code = 0;
    }
    return code;
}

unicode_t sym_key_to_unicode(Unicode_SymType sym, const QKeyEvent & evt)
{
    unicode_t code;
    if(evt.type() == QEvent::KeyPress)
    {
        switch(sym)
        {
            case UNICODE_SYM_TYPE_BASIC_LATIN: code = basic_latin_key_to_unicode(evt); break;
            case UNICODE_SYM_TYPE_CYRILLIC:    code = cyrillic_key_to_unicode(evt);    break;
            case UNICODE_SYM_TYPE_DIACRITIC:   code = diacritic_key_to_unicode(evt);   break;
            default: code = 0;
        }
    }
    else
        code = 0;
    return code;
}

unicode_t ctrl_simple_key_to_unicode(const QKeyEvent & evt, unicode_t code)
{
    return evt.type() == QEvent::KeyPress ? code : 0;
}

unicode_t ctrl_switchable_key_to_unicode(const QKeyEvent & evt, unicode_t code)
{
    return evt.type() == QEvent::KeyRelease ? code | 0x80 : code;
}

unicode_t ctrl_activatable_key_to_unicode(int vk, unicode_t code)
{
    auto st = GetKeyState(vk);
    if(st & 0x80)
        code = (st & 0x01) ? code | 0x80 : code;
    else
        code = 0;
    return code;
}

unicode_t basic_latin_key_to_unicode(const QKeyEvent & evt)
{
    auto key = evt.key();
    unicode_t code = (unicode_t)key;

    if(key >= 0x41 && key <= 0x5A)
        if(check_for_not_capital(evt.modifiers()))
            code += 0x20;

    return code;
}

unicode_t cyrillic_key_to_unicode(const QKeyEvent & evt)
{
    auto key = evt.key();
    auto mod = evt.modifiers();
    unicode_t code = (unicode_t)key;

    if(key == 0x0401)
        if(check_for_not_capital(mod))
            code = 0x0451;

    if(key >= 0x0410 && key <= 0x042F)
        if(check_for_not_capital(mod))
            code += 0x20;

    return code;
}

unicode_t diacritic_key_to_unicode(const QKeyEvent & evt)
{
    return (unicode_t)evt.key();
}

QByteArray unicode_to_hex_array(unicode_t code)
{
    QByteArray arr(2, 0);
    arr[1] = (uint8_t)(uint16_t)code;
    arr[0] = (uint8_t)((uint16_t)code >> 8);
    arr = arr.toHex();
    arr.insert(0, '+');
    arr.insert(0, 'U');
    return arr;
}

unicode_t ctrl_key_to_unicode(int key, Qt::KeyboardModifiers mod)
{
    switch(key)
    {
        case Qt::Key_Control:  return (unicode_t)(mod & Qt::ControlModifier ? UNICODE_CTRL_P : UNICODE_CTRL_R);
        case Qt::Key_Alt:      return (unicode_t)(mod & Qt::AltModifier ? UNICODE_ALT_P : UNICODE_ALT_R);
        case Qt::Key_Shift:    return (unicode_t)(mod & Qt::ShiftModifier ? UNICODE_SHIFT_P : UNICODE_SHIFT_R);

        case Qt::Key_CapsLock: return check_key_active(VK_CAPITAL) ? UNICODE_CAPS_A : UNICODE_CAPS_N;
        case Qt::Key_Insert:   return check_key_active(VK_INSERT) ? UNICODE_INSERT_A : UNICODE_INSERT_N;
        case Qt::Key_F1:       return check_key_active(VK_F1) ? UNICODE_LANG_A : UNICODE_LANG_N;

        case Qt::Key_Left:  return UNICODE_LEFT;
        case Qt::Key_Up:    return UNICODE_UP;
        case Qt::Key_Right: return UNICODE_RIGHT;
        case Qt::Key_Down:  return UNICODE_DOWN;

        case Qt::Key_Escape:    return UNICODE_ESC;
        case Qt::Key_Tab:       return UNICODE_TAB;
        case Qt::Key_Backspace: return UNICODE_BACKSPACE;
        case Qt::Key_Enter:     return UNICODE_ENTER;
        case Qt::Key_Return:    return UNICODE_ENTER;
        case Qt::Key_Delete:    return UNICODE_DEL;

        case Qt::Key_Home:     return UNICODE_HOME;
        case Qt::Key_End:      return UNICODE_END;
        case Qt::Key_PageUp:   return UNICODE_PGUP;
        case Qt::Key_PageDown: return UNICODE_PGDN;
    }
    return 0;
}

unicode_t ascii_key_to_unicode(int key, Qt::KeyboardModifiers mod)
{
    if(key >= 0x20 && key <= 0x7E)
    {
        if(key >= 0x41 && key <= 0x5A)
        {
            if(check_for_not_capital(mod))
                return (unicode_t)key + 0x20;
        }
        return (unicode_t)key;
    }

    return 0;
}

unicode_t cyrillic_key_to_unicode(int key, Qt::KeyboardModifiers mod)
{
    // Буква "Ё"
    if(key == 0x0401)
        return check_for_not_capital(mod) ? (unicode_t)0x451 : (unicode_t)0x401;

    if(key >= 0x410 && key <= 0x42F)
        return check_for_not_capital(mod) ? (unicode_t)(key + 0x20) : (unicode_t)key;

    return 0;
}

bool check_key_active(int vkKey)
{
    return GetKeyState(vkKey) & 1;
}

bool check_for_not_capital(Qt::KeyboardModifiers mod)
{
    if(check_key_active(VK_CAPITAL))
    {
        if(mod & Qt::ShiftModifier)
            return true;
    }
    else
    {
        if(!(mod & Qt::ShiftModifier))
            return true;
    }
    return false;
}

TestKeyboardKeyCatcher::TestKeyboardKeyCatcher()
    : QObject()
{}

void TestKeyboardKeyCatcher::key_event(int code)
{
    this->code = code;
    emit _key_catched();
}
