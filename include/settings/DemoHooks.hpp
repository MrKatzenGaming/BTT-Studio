#pragma once

#include <hk/hook/Trampoline.h>

#include "src/settings/SettingsMgr.h"

#define HOOK_CALLBACK(NAME)                                                                                  \
    inline HkTrampoline NAME = [](TrampolineStatic(), void* thisPtr) -> bool {                               \
        if (btt::SettingsMgr::instance()->getSettings()->mIsEnableAlwaysManualCutscene) return true;         \
        return orig(thisPtr);                                                                                \
    };

HOOK_CALLBACK(RsDemoHook);
HOOK_CALLBACK(FirstDemoScenarioHook);
HOOK_CALLBACK(FirstDemoWorldHook);
HOOK_CALLBACK(FirstDemoMoonRockHook);
HOOK_CALLBACK(ShowDemoHackHook);

inline void installDemoHooks() {
    RsDemoHook.installAtSym<"_ZN2rs11isFirstDemoEPKN2al5SceneE">();
    FirstDemoScenarioHook.installAtSym<"_ZN2rs30isFirstDemoScenarioStartCameraEPKN2al9LiveActorE">();
    FirstDemoWorldHook.installAtSym<"_ZN2rs27isFirstDemoWorldIntroCameraEPKN2al5SceneE">();
    FirstDemoMoonRockHook.installAtSym<"_ZNK12MoonRockData38isEnableShowDemoAfterOpenMoonRockFirstEv">();
    ShowDemoHackHook.installAtSym<"_ZNK18DemoStateHackFirst20isEnableShowHackDemoEv">();
}
