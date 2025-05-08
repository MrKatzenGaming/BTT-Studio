#pragma once

#include <sead/heap/seadDisposer.h>

#include "imgui.h"

namespace btt {

#define SETTING(NAME) bool mIsEnable##NAME;
#define VERSION "1.2.2a"
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

    enum class InputDisplayColor : u8 { None, White, Gray, Black, Red, Blue, Yellow, Green, Orange, Purple, Pink, LightBlue };

    struct Settings {
        char Version[16] = VERSION;
        SETTING(MoonRefresh);
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
        InputDisplayColor mInputDisplayButtonColor = InputDisplayColor::White;
        InputDisplayColor mInputDisplayButtonPressedColor = InputDisplayColor::Pink;
        InputDisplayColor mInputDisplayStickColor = InputDisplayColor::White;
        InputDisplayColor mInputDisplayRingColor = InputDisplayColor::Gray;
        InputDisplayColor mInputDisplayBackColor = InputDisplayColor::Black;
        ImVec2 mInputDisplayPos = ImVec2(1600.f / 2, 900.f / 2);
        int mKillSceneKey = 0;
        int mHealMarioKey = 0;
        int mMoonRefreshText = 0;
        int mPrevSceneKey = 0;
        int mIncTpIndexKey = 0;
        int mDecTpIndexKey = 0;
        int mAddCoinsKey = 0;
        int mDecCoinsKey = 0;
        int mLifeUpKey = 0;
        int mWigglerPattern = 0;
        int mIncPatternKey = 0;
        int mDecPatternKey = 0;
        int mMenuCorner = 0;
        ImVec2 mInfoPos = ImVec2(0, 0);
        int mWarpLastCpKey = 0;
        SETTING(Input2P);
        ImVec2 mInputDisplayPosP2 = ImVec2(1600.f / 2, 900.f / 2);
        SETTING(HintPhotoSpawn);

    } mSettings;

    Settings* getSettings() { return &mSettings; }
};

} // namespace btt
