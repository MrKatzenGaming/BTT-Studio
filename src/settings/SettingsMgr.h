#pragma once

#include <heap/seadDisposer.h>
#include "settings/SettingsHooks.h"

namespace btt {

#define SETTING(NAME) bool mIsEnable##NAME;

class SettingsMgr {
    SEAD_SINGLETON_DISPOSER(SettingsMgr)

private:
    sead::Heap* mHeap;
public:
    SettingsMgr() = default;

    struct Settings {
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
    } mSettings;

    void loadSettings();
    void saveSettings();

    // Settings* mSettings = nullptr;
    Settings* getSettings() { return &mSettings; }
    char* mPath = "sd:/BTT-Studio/Settings.bin";
    };



} // namespace btt