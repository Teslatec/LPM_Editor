#ifndef TEST_EDITOR_SW_SUPPORT_H
#define TEST_EDITOR_SW_SUPPORT_H

extern "C"
{
#include "lpm_editor_api.h"
}

class TestEditorSwSupport
{
public:
    static bool readSettings
            (LPM_EditorSettings * setting);

    static bool readSupportFxns
            ( LPM_SupportFxns * fxns,
              LPM_Lang lang );

    static bool readGuiText
            ( Unicode_Buf * text,
              LPM_Lang lang,
              LPM_GuiTextId id );
};

#endif // TEST_EDITOR_SW_SUPPORT_H
