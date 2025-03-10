#include "SettingsHooks.h"

#include "SettingsMgr.h"
#include "al/Library/Bgm/BgmLineFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "game/System/GameDataHolderWriter.h"
#include "getHelper.h"
#include "helpers.h"
#include "hk/hook/Trampoline.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/detail.h"
#include "game/System/PlayerHitPointData.h"
#include "settings/DemoHooks.hpp"

using namespace hk;
using namespace btt;

class ShineInfo;
HkTrampoline<bool, GameDataHolderWriter, const ShineInfo*> GreyShineRefreshHook =
    hk::hook::trampoline([](GameDataHolderWriter writer, const ShineInfo* shineInfo) -> bool {
        return SettingsMgr::instance()->mSettings.mIsEnableMoonRefresh ? false : GreyShineRefreshHook.orig(writer, shineInfo);
    });

HkTrampoline<void, GameDataHolderWriter, const ShineInfo*> ShineRefreshHook =
    hk::hook::trampoline([](GameDataHolderWriter writer, const ShineInfo* shineInfo) -> void {
        ptr addr = sail::lookupSymbolFromDb<>("MoonRefreshText");
        ptr offset = addr - ro::getMainModule()->range().start();
        ro::getMainModule()->writeRo(offset, "BTT Studio", 12);

        if (!SettingsMgr::instance()->mSettings.mIsEnableMoonRefresh) ShineRefreshHook.orig(writer, shineInfo);
    });

HkTrampoline<void, al::LiveActor*> marioControl = hk::hook::trampoline([](al::LiveActor* player) -> void { marioControl.orig(player); });

HkTrampoline<void, StageScene*> stageSceneControlHook = hk::hook::trampoline([](StageScene* stageScene) -> void {
    PlayerActorHakoniwa* player = helpers::tryGetPlayerActorHakoniwa(stageScene);

    if (SettingsMgr::instance()->mSettings.mIsEnableDisableMusic) {
        if (al::isPlayingBgm(stageScene)) al::stopAllBgm(stageScene, 0);
    }

    stageSceneControlHook.orig(stageScene);
});

HkTrampoline<bool, StageScene*> saveHook =
    hk::hook::trampoline([](StageScene* scene) -> bool { return SettingsMgr::instance()->mSettings.mIsEnableDisableAutoSave ? false : saveHook.orig(scene); });

HkTrampoline<bool, void*> checkpointWarpHook = hk::hook::trampoline([](void* thisPtr) -> bool {
    return SettingsMgr::instance()->mSettings.mIsEnableAlwaysCheckpoints ? true : checkpointWarpHook.orig(thisPtr);
});

HkTrampoline<int, GameDataHolder*, bool*, int> disableMoonLockHook = hk::hook::trampoline([](GameDataHolder* thisPtr, bool* isCrashList, int worldID) -> int {
    int lockSize = disableMoonLockHook.orig(thisPtr, isCrashList, worldID);

    return SettingsMgr::instance()->mSettings.mIsEnableDisableMoonLock ? 0 : lockSize;
});

// HOOK_DEFINE_TRAMPOLINE(NoDamageHook){
//     static void Callback(PlayerHitPointData* hitPointData) {
//         if (!DevGuiManager::instance()->getSettings()->getStateByName("No Damage"))
//             return Orig(hitPointData);
//     }
// };

HkTrampoline<void, PlayerHitPointData*> NoDamageHook = hk::hook::trampoline([](PlayerHitPointData* hitPointData) -> void {
    if (!SettingsMgr::instance()->mSettings.mIsEnableNoDamage) 
        NoDamageHook.orig(hitPointData);
    
});

void SettingsHooks::installSettingsHooks() {

    installDemoHooks();

    GreyShineRefreshHook.installAtSym<"_ZN16GameDataFunction10isGotShineE22GameDataHolderAccessorPK9ShineInfo">();
    ShineRefreshHook.installAtSym<"_ZN16GameDataFunction11setGotShineE20GameDataHolderWriterPK9ShineInfo">();
    marioControl.installAtSym<"_ZN19PlayerActorHakoniwa7controlEv">();
    stageSceneControlHook.installAtSym<"_ZN10StageScene7controlEv">();
    saveHook.installAtSym<"_ZNK10StageScene12isEnableSaveEv">();
    checkpointWarpHook.installAtSym<"_ZNK9MapLayout22isEnableCheckpointWarpEv">();
    disableMoonLockHook.installAtSym<"_ZNK14GameDataHolder18findUnlockShineNumEPbi">();
    NoDamageHook.installAtSym<"_ZN16GameDataFunction12damagePlayerE20GameDataHolderWriter">();
}
