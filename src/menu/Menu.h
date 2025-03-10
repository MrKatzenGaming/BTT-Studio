#pragma once

#include <heap/seadDisposer.h>

// #include "hk/gfx/DebugRenderer.h"
#include "imgui.h"
#include "sead/gfx/seadDrawContext.h"
#include "settings/SettingsMgr.h"

namespace btt {

class Menu {
    SEAD_SINGLETON_DISPOSER(Menu)

private:
    SettingsMgr::Settings mSettings;

    void drawInputDisabled();

    ImVec2 mWindowSize = ImVec2(500, 400);

public:
    Menu() = default;
    bool mIsEnabledMenu = true;

    void draw();
    void setupStyle();
};

} // namespace btt
