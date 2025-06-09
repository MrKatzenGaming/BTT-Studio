#pragma once

#include <sead/heap/seadDisposer.h>
#include <sead/math/seadQuat.h>

#include <game/Player/PlayerActorBase.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/Scene/StageScene.h>
#include <game/Sequence/HakoniwaSequence.h>

#include <cstdio>

#include "imgui.h"
#include "settings/SettingsMgr.h"

class WorldTravelingNpc;

namespace btt {

class Menu {
    SEAD_SINGLETON_DISPOSER(Menu)

public:
    Menu() = default;
    void draw();
    void handleAlways();
    void drawInfoWindow();
    void drawPopup();

    void setLatestMapTarget(void* target) { mLastMapTarget = target; }

    bool isPatternReverse();
    int getPatternTarget(int a);

    const char* getMoonRefreshText();

public:
    struct TpState {
        bool saved;
        sead::Vector3f pos = { 0, 0, 0 };
        sead::Quatf quat = { 0, 0, 0, 0 };
        char stageName[0x40] = {};
    };

    TpState tpStates[0x8];

    bool mIsEnableNoclip = false;
    bool noGetPlayer = false;
    bool mIsEnabledMenu = true;
    WorldTravelingNpc* mWorldTravelingNpc = nullptr;
    const char* mWorldTravelingStatus = nullptr;

private:
    void drawPageOptions();
    void drawPageInputDisplay();
    void drawPageHotkeys();
    void drawPageInfo();
    void drawPageMisc();
    void drawTeleportCat();

    void saveTeleport(TpState& state);
    void loadTeleport(TpState& state);
    void handleHotkeys();
    void handlePopup();

    void setPopupText(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(popupText, sizeof(popupText), fmt, args);
        va_end(args);
        mIsPopup = true;
        popupTimer = 0;
    }

    bool isHotkey(int key);

    ImVec2 getCornerPos(int corner);

    void drawComplexHeapTreeItem(sead::Heap* heap);

private:
    ImVec2 mWindowSize = ImVec2(500, 400);
    ImGuiID prevNavId = 0;
    u64 menuTimer = 0;
    u64 popupTimer = 0;

    HakoniwaSequence* gameSeq;
    StageScene* stageScene;
    PlayerActorHakoniwa* playerHak;
    SettingsMgr* set;

    void* mLastMapTarget = nullptr;

    bool mIsPopup = false;
    bool isEnableInfoWindow = false;
    bool isEnablePlayerInfo = false;
    bool mIsReloadPos = false;
    int reloadPosTimer = -1;
    int mSelectedBody = 0;
    int mSelectedCap = 0;
    int tpIndex = 0;
    sead::Vector3f reloadStagePos = { 0, 0, 0 };
    sead::Quatf reloadStageQuat = { 0, 0, 0, 0 };

    char popupText[0x40] = "Input Disabled";

    constexpr static const char* Corners[4] { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };
    constexpr static const char* MoonRefreshTexts[] { "BTT Studio", "BTT", "Hi BTT", "I am Cool", "Super Mario Odyssey", "Entrance to Shiveria", "<blank>" };
    constexpr static const char* WigglerPatterns[23] { "Random", "Ghost",   "Nose",    "C",        "W",     "J", "Medal",    "Plane",
                                                       "5",      "Hangman", "Spanish", "Siblings", "Snake", "8", "Mushroom", "Z",
                                                       "Tetris", "Ear",     "Bomb",    "Bird",     "L",     "O", "Star" };
    constexpr static const char* worldTravelingStatus[] = { "<Normal>", "First Talk", "Metro",     "Cascade",        "Luncheon",
                                                            "Moon",     "Mushroom",   "Last Talk", "After Last Talk" };

    constexpr static const char* Keys[] {
        "Disabled", "None", "L", "R", "ZL", "ZR", "L + R", "L + ZL", "L + ZR", "R + ZL", "R + ZR",
    };

    constexpr static const char* bodyNames[] = {
        "Mario",          "Mario64",         "Mario64Metal",      "MarioAloha",      "MarioArmor",     "MarioBone",      "MarioClown",    "MarioColorClassic",
        "MarioColorGold", "MarioColorLuigi", "MarioColorWaluigi", "MarioColorWario", "MarioCook",      "MarioDiddyKong", "MarioDoctor",   "MarioExplorer",
        "MarioFootball",  "MarioGolf",       "MarioGunman",       "MarioHakama",     "MarioHappi",     "MarioKing",      "MarioKoopa",    "MarioMaker",
        "MarioMechanic",  "MarioNew3DS",     "MarioPainter",      "MarioPeach",      "MarioPilot",     "MarioPirate",    "MarioPoncho",   "MarioPrimitiveMan",
        "MarioSailor",    "MarioScientist",  "MarioShopman",      "MarioSnowSuit",   "MarioSpaceSuit", "MarioSuit",      "MarioSwimwear", "MarioTailCoat",
        "MarioTuxedo",    "MarioUnderwear"
    };

    constexpr static const char* capNames[] = {
        "Mario",          "Mario64",         "Mario64Metal",      "MarioAloha",      "MarioArmor",     "MarioBone",      "MarioClown",    "MarioColorClassic",
        "MarioColorGold", "MarioColorLuigi", "MarioColorWaluigi", "MarioColorWario", "MarioCook",      "MarioDiddyKong", "MarioDoctor",   "MarioExplorer",
        "MarioFootball",  "MarioGolf",       "MarioGunman",       "MarioHakama",     "MarioHappi",     "MarioKing",      "MarioKoopa",    "MarioMaker",
        "MarioMechanic",  "MarioNew3DS",     "MarioPainter",      "MarioPeach",      "MarioPilot",     "MarioPirate",    "MarioPoncho",   "MarioPrimitiveMan",
        "MarioSailor",    "MarioScientist",  "MarioShopman",      "MarioSnowSuit",   "MarioSpaceSuit", "MarioSuit",      "MarioSwimwear", "MarioTailCoat",
        "MarioTuxedo",    "MarioInvisible"
    };

    constexpr static const char* TimerHookTypes[] = {
        "None", "Shine Grab (Kinda Broken)", "Shine Tick", "Game Start (Not Implemented)", "Escape Wire (Not Implemented)", "Capture Glow (Not Implemented)",
    };
};
} // namespace btt
