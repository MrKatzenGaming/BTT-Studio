#pragma once

#include <heap/seadDisposer.h>

// #include "hk/gfx/DebugRenderer.h"
#include "imgui.h"
#include "sead/gfx/seadDrawContext.h"
#include "settings/SettingsMgr.h"
#include "sead/math/seadVector.h"
#include "sead/math/seadQuat.h"

namespace btt {

class Menu {
    SEAD_SINGLETON_DISPOSER(Menu)

private:
    SettingsMgr::Settings mSettings;
    ImVec2 mWindowSize = ImVec2(500, 400);

    void drawInputDisabled();
    void drawMiscCat();
    void drawTeleportCat();
        struct TpState {
            bool saved;
            sead::Vector3f pos = {0, 0, 0};
            sead::Quatf quat = {0, 0, 0, 0};
        };
        TpState tpStates[0x8];
        int tpIndex = 0;
        bool mIsEnabledTpHotkeys = false;
        void saveTeleport(TpState& state);
        void loadTeleport(TpState& state);

public:
    Menu() = default;
    bool mIsEnabledMenu = true;

    void draw();
    void setupStyle();
    void handleInput();
};

} // namespace btt
