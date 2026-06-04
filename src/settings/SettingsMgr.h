#pragma once

#include <sead/heap/seadDisposer.h>

#include "imgui.h"
#include "menu/Timer.h"

namespace btt {

#define SETTING(NAME) bool mIsEnable##NAME
#define VERSION "1.2.4"
#ifdef BTTDEBUG
# define SUBVER "-dev"
#else
# define SUBVER ""
#endif

class SettingsMgr {
    SEAD_SINGLETON_DISPOSER(SettingsMgr)

private:
    sead::Heap* mHeap;

public:
    SettingsMgr() = default;

    enum class InputDisplayColor : u8 {
        None,
        White,
        Gray,
        Black,
        Red,
        Blue,
        Yellow,
        Green,
        Orange,
        Purple,
        Pink,
        LightBlue
    };
    enum class SettingsHotkey : u8 { Disabled, None, L, R, ZL, ZR, LR, LZL, LZR, RZL, RZR };

    struct __attribute__((packed)) Settings {
        char Version[0x8] = VERSION;
        SETTING(MoonRefresh);
        SETTING(GrayMoonRefresh);
        SETTING(NoDamage);
        SETTING(DisableMoonLock);
        SETTING(AlwaysManualCutscene);
        SETTING(DisableAutoSave);
        SETTING(DisableMusic);
        SETTING(AlwaysCheckpoints);
        SETTING(RefreshWarpText);
        SETTING(RefreshKingdomEnter);
        SETTING(TpHotkeys);
        SETTING(DisableTpPuppet);
        SETTING(RefreshPurps);
        SETTING(InputDisplay);
        SETTING(DoorRefresh);
        SETTING(ShardRefresh);
        SETTING(FlowerPotRefresh);
        SETTING(NoCheckpointTouch);
        SETTING(SkipCloud);
        SETTING(AllCheckpoints);
        SETTING(Input2P);
        SETTING(HintPhotoSpawn);
        SETTING(SegmentTimer);
        SETTING(SegTimerSplit);
        SETTING(RefreshNpc);
        SETTING(DisableCoinStackSave);
        InputDisplayColor mInputDisplayButtonColor = InputDisplayColor::White;
        InputDisplayColor mInputDisplayButtonPressedColor = InputDisplayColor::Pink;
        InputDisplayColor mInputDisplayStickColor = InputDisplayColor::White;
        InputDisplayColor mInputDisplayRingColor = InputDisplayColor::Gray;
        InputDisplayColor mInputDisplayBackColor = InputDisplayColor::Black;
        SettingsHotkey mKillSceneKey = SettingsHotkey::Disabled;
        SettingsHotkey mHealMarioKey = SettingsHotkey::Disabled;
        SettingsHotkey mPrevSceneKey = SettingsHotkey::Disabled;
        SettingsHotkey mIncTpIndexKey = SettingsHotkey::Disabled;
        SettingsHotkey mDecTpIndexKey = SettingsHotkey::Disabled;
        SettingsHotkey mAddCoinsKey = SettingsHotkey::Disabled;
        SettingsHotkey mDecCoinsKey = SettingsHotkey::Disabled;
        SettingsHotkey mLifeUpKey = SettingsHotkey::Disabled;
        SettingsHotkey mIncPatternKey = SettingsHotkey::Disabled;
        SettingsHotkey mDecPatternKey = SettingsHotkey::Disabled;
        SettingsHotkey mTimerStartKey = SettingsHotkey::Disabled;
        SettingsHotkey mTimerEndKey = SettingsHotkey::Disabled;
        SettingsHotkey mTimerResetKey = SettingsHotkey::Disabled;
        SettingsHotkey mWarpLastCpKey = SettingsHotkey::Disabled;
        TimerHookType mTimerStartType = TimerHookType::None;
        TimerHookType mTimerEndType = TimerHookType::None;
        u8 mWorldTravelingStatus = 0;
        u8 mMenuCorner = 0;
        u8 mMenuKey = 0;
        u8 mMoonRefreshText = 0;
        u8 mWigglerPattern = 0;
        ImVec2 mInfoPos = ImVec2(0, 0);
        ImVec2 mInputDisplayPosP2 = ImVec2(1600.f / 2, 900.f / 2);
        ImVec2 mTimerPos = ImVec2(1600.f / 2, 900.f / 2);
        ImVec2 mInputDisplayPos = ImVec2(1600.f / 2, 900.f / 2);
    } mSettings;

    // Settings* getSettings() { return &mSettings; }

    static Settings* getSettings() { return sInstance ? &sInstance->mSettings : nullptr; }
};

} // namespace btt
