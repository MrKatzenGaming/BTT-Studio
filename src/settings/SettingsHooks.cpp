#include "SettingsHooks.h"

#include "Menu.h"
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
#include "al/Library/Nerve/NerveUtil.h"

using namespace hk;
using namespace btt;

class ShineInfo;
HkTrampoline<bool, GameDataHolderWriter, const ShineInfo*> GreyShineRefreshHook =
    hk::hook::trampoline([](GameDataHolderWriter writer, const ShineInfo* shineInfo) -> bool {
        return SettingsMgr::instance()->getSettings()->mIsEnableMoonRefresh ? false : GreyShineRefreshHook.orig(writer, shineInfo);
    });

HkTrampoline<void, GameDataHolderWriter, const ShineInfo*> ShineRefreshHook =
    hk::hook::trampoline([](GameDataHolderWriter writer, const ShineInfo* shineInfo) -> void {
        ptr addr = sail::lookupSymbolFromDb<>("MoonRefreshText");
        ptr offset = addr - ro::getMainModule()->range().start();
        const char* text = Menu::instance()->getMoonRefreshText();
        ro::getMainModule()->writeRo(offset, text, strlen(text) + 1);

        if (!SettingsMgr::instance()->getSettings()->mIsEnableMoonRefresh) ShineRefreshHook.orig(writer, shineInfo);
    });

HkTrampoline<void, al::LiveActor*> marioControl = hk::hook::trampoline([](al::LiveActor* player) -> void { marioControl.orig(player); });

HkTrampoline<void, StageScene*> stageSceneControlHook = hk::hook::trampoline([](StageScene* stageScene) -> void {

    if (SettingsMgr::instance()->getSettings()->mIsEnableDisableMusic) {
        if (al::isPlayingBgm(stageScene)) al::stopAllBgm(stageScene, 0);
    }

    stageSceneControlHook.orig(stageScene);
});

HkTrampoline<bool, StageScene*> saveHook =
    hk::hook::trampoline([](StageScene* scene) -> bool { return SettingsMgr::instance()->getSettings()->mIsEnableDisableAutoSave ? false : saveHook.orig(scene); });

HkTrampoline<bool, void*> checkpointWarpHook = hk::hook::trampoline([](void* thisPtr) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableAlwaysCheckpoints ? true : checkpointWarpHook.orig(thisPtr);
});

HkTrampoline<int, GameDataHolder*, bool*, int> disableMoonLockHook = hk::hook::trampoline([](GameDataHolder* thisPtr, bool* isCrashList, int worldID) -> int {
    int lockSize = disableMoonLockHook.orig(thisPtr, isCrashList, worldID);

    return SettingsMgr::instance()->getSettings()->mIsEnableDisableMoonLock ? 0 : lockSize;
});

HkTrampoline<void, PlayerHitPointData*> NoDamageHook = hk::hook::trampoline([](PlayerHitPointData* hitPointData) -> void {
    if (!SettingsMgr::instance()->getSettings()->mIsEnableNoDamage) 
        NoDamageHook.orig(hitPointData);
    
});

HkTrampoline<bool,  GameDataHolderAccessor> kingdomEnterHook = hk::hook::trampoline([]( GameDataHolderAccessor accessor) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableRefreshKingdomEnter ? false : kingdomEnterHook.orig(accessor);
});

HkTrampoline<bool, GameDataHolderAccessor> warpTextHook = hk::hook::trampoline([](GameDataHolderAccessor accessor) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableRefreshWarpText ? false : warpTextHook.orig(accessor);
});

HkTrampoline<bool, GameDataHolderAccessor, al::ActorInitInfo*> refreshPurpsHook = hk::hook::trampoline([](GameDataHolderAccessor accessor, al::ActorInitInfo* actorInitInfo) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableRefreshPurps ? false : refreshPurpsHook.orig(accessor, actorInitInfo);
});
class DoorAreaChange;
HkTrampoline<bool, DoorAreaChange*> doorRefreshHook = hk::hook::trampoline([](DoorAreaChange* doorAreaChange) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableDoorRefresh ? false : doorRefreshHook.orig(doorAreaChange);
});
class ShineChip;
HkTrampoline<bool, ShineChip*> shardRefreshHook = hk::hook::trampoline([](ShineChip* shineChip) -> bool {
    al::setNerve((al::IUseNerve*)shineChip, (al::Nerve*)(hk::ro::getMainModule()->range().start() + 0x1cbeaf8));
    return SettingsMgr::instance()->getSettings()->mIsEnableShardRefresh ? false : shardRefreshHook.orig(shineChip);
});
class GrowFlowerPot;
HkTrampoline<bool, GrowFlowerPot*> flowerPotRefreshHook = hk::hook::trampoline([](GrowFlowerPot* flowerPot) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableFlowerPotRefresh ? false : flowerPotRefreshHook.orig(flowerPot);
});
class CheckpointFlag;
HkTrampoline<void, CheckpointFlag*> checkpointFlagHook = hk::hook::trampoline([](CheckpointFlag* checkpointFlag) -> void {
    if (!SettingsMgr::instance()->getSettings()->mIsEnableNoCheckpointTouch)
        checkpointFlagHook.orig(checkpointFlag);
});
HkTrampoline<bool, StageScene*> cloudSkipHook = hk::hook::trampoline([](StageScene* stageScene) -> bool {
    Menu::instance()->noGetPlayer = true;
    static int functionCalls = 0;
    if (!SettingsMgr::instance()->getSettings()->mIsEnableSkipCloud) {
        functionCalls = 0;
        Menu::instance()->noGetPlayer = false;
        return cloudSkipHook.orig(stageScene);
    }
    // When debugging, returning true right away will skip Lost and go straight to (Day!) Metro with a broken Odyssey back in Wooded.
    // Since isDefeatKoopaLv1 is called multiple times, we can return false the first time, then return true the second time, which
    // seems to correctly skip the Bowser fight. This is a very hacky way of skipping the Bowser fight, so if there is some way to
    // deduce from other game variables which function call we're in, that would be ideal. It would eliminate the need to introduce
    // our own variable to keep track of the number of isDefeatKoopaLv1 calls.
    functionCalls++;
    if (functionCalls == 2) {
        functionCalls = 0;
        return true;
    }
    Menu::instance()->noGetPlayer = false;

    return false;
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
    warpTextHook.installAtSym<"_ZN16GameDataFunction34isAlreadyShowExplainCheckpointFlagE22GameDataHolderAccessor">();
    refreshPurpsHook.installAtSym<"_ZN16GameDataFunction16isGotCoinCollectE22GameDataHolderAccessorRKN2al13ActorInitInfoE">();
    // doorRefreshHook.installAtSym<"_ZNK14DoorAreaChange6isOpenEv">();
    // shardRefreshHook.installAtSym<"_ZNK9ShineChip5isGotEv">();
    // kingdomEnterHook.installAtSym<"_ZN16GameDataFunction11isGameClearE22GameDataHolderAccessor">();
    // flowerPotRefreshHook.installAtSym<"_ZNK13GrowFlowerPot12isEnableGrowEv">();
    checkpointFlagHook.installAtSym<"_ZN2rs31setTouchCheckpointFlagToWatcherEP14CheckpointFlag">();
    cloudSkipHook.installAtSym<"_ZNK10StageScene16isDefeatKoopaLv1Ev">();

}
