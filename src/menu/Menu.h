#pragma once

#include <heap/seadDisposer.h>

#include "game/Player/PlayerActorBase.h"
#include "game/Scene/StageScene.h"
#include "game/Sequence/HakoniwaSequence.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "imgui.h"
#include "sead/math/seadQuat.h"
#include "settings/SettingsMgr.h"

namespace btt {

class Menu {
    SEAD_SINGLETON_DISPOSER(Menu)

public:
    Menu() = default;
    bool mIsEnabledMenu = true;

    struct TpState {
        bool saved;
        sead::Vector3f pos = { 0, 0, 0 };
        sead::Quatf quat = { 0, 0, 0, 0 };
        char stageName[0x40] = {};
    };

    TpState tpStates[0x8];

    void draw();
    void setupStyle();
    void handleAlways();

private:
    ImVec2 mWindowSize = ImVec2(500, 400);

    HakoniwaSequence* gameSeq;
    PlayerActorBase* player;
    StageScene* stageScene;
    PlayerActorHakoniwa* playerHak;
    GameDataHolder* holder;
    SettingsMgr* set;

    void drawInputDisabled();
    void drawMiscCat();
    void drawTeleportCat();

    int tpIndex = 0;
    void saveTeleport(TpState& state);
    void loadTeleport(TpState& state);
};
} // namespace btt
