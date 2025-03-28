#pragma once

#include <heap/seadDisposer.h>

#include "game/Player/PlayerActorBase.h"
#include "game/Scene/StageScene.h"
#include "game/Sequence/HakoniwaSequence.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "imgui.h"
#include "sead/math/seadQuat.h"
#include "settings/SettingsMgr.h"
#include "sead/heap/seadExpHeap.h"

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
    const char* getMoonRefreshText();

    bool noGetPlayer = false;
    u64 globalTimer = 0;

    bool isPatternReverse();
    int getPatternTarget(int a);

    bool mIsEnableNoclip = false;
private:
    ImVec2 mWindowSize = ImVec2(500, 400);

    HakoniwaSequence* gameSeq;
    PlayerActorBase* player;
    StageScene* stageScene;
    PlayerActorHakoniwa* playerHak;
    SettingsMgr* set;

    ImGuiID prevNavId = 0;
    u64 prevTime = 0;

    void drawInputDisabled();
    void drawMiscCat();
        constexpr static const char* MoonRefreshTexts[] {
            "BTT Studio",
            "BTT",
            "Hi BTT",
            "I am Cool",
            "Super Mario Odyssey",
            "Entrance to Shiveria",
            "<blank>"
        };
        constexpr static const char* WigglerPatterns[23] {"Random", "Ghost", "Nose", "C", "W",
            "J", "Medal", "Plane", "5",
            "Hangman", "Spanish", "Siblings", "Snake", "8", "Mushroom", "Z", "Tetris",
            "Ear", "Bomb", "Bird", "L", "O", "Star"
        };
        constexpr static const char* Corners[4] {"Top Left", "Top Right", "Bottom Left", "Bottom Right"};
        ImVec2 getCornerPos(int corner);
    void drawTeleportCat();
    void drawHotkeysCat();
    bool isHotkey(int key);
        constexpr static const char* Keys[] {
            "Disabled",
            "None",
            "L",
            "R",
            "ZL",
            "ZR",
            "L + R",
            "L + ZL",
            "L + ZR",
            "R + ZL",
            "R + ZR",
        };
        int killSceneKey = 0;
        int healMarioKey = 0;
    void drawInfoCat();
        void drawInfoWindow();
        bool isEnableInfoWindow = false;
        bool isEnablePlayerInfo = false;

    int tpIndex = 0;
    void saveTeleport(TpState& state);
    void loadTeleport(TpState& state);

    bool prevMouseDis = true;

};
} // namespace btt
