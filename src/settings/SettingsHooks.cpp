#include "SettingsHooks.h"

#include "hk/hook/InstrUtil.h"
#include "hk/hook/Trampoline.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/detail.h"

#include <al/Library/Bgm/BgmLineFunction.h>
#include <al/Library/Camera/CameraUtil.h>
#include <al/Library/LiveActor/ActorPoseUtil.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Nerve/NerveKeeper.h>
#include <al/Library/Nerve/NerveUtil.h>

#include "game/System/GameDataFile.h"
#include <game/Item/CoinStackGroup.h>
#include <game/MapObj/DoorAreaChange.h>
#include <game/System/GameDataFunction.h>
#include <game/System/GameDataHolder.h>
#include <game/System/GameDataHolderWriter.h>
#include <game/System/GameDataUtil.h>
#include <game/System/PlayerHitPointData.h>

#include "InputHelper.h"
#include "Menu.h"
#include "settings/DemoHooks.hpp"
#include "SettingsMgr.h"

using namespace hk;
using namespace btt;

class ShineInfo;
class ShineChip;
class GrowFlowerPot;
class CheckpointFlag;

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

HkTrampoline<bool, StageScene*> saveHook = hk::hook::trampoline([](StageScene* scene) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableDisableAutoSave ? false : saveHook.orig(scene);
});

HkTrampoline<bool, void*> checkpointWarpHook = hk::hook::trampoline([](void* thisPtr) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableAlwaysCheckpoints ? true : checkpointWarpHook.orig(thisPtr);
});

HkTrampoline<int, GameDataHolder*, bool*, int> disableMoonLockHook = hk::hook::trampoline([](GameDataHolder* thisPtr, bool* isCrashList, int worldID) -> int {
    int lockSize = disableMoonLockHook.orig(thisPtr, isCrashList, worldID);

    return SettingsMgr::instance()->getSettings()->mIsEnableDisableMoonLock ? 0 : lockSize;
});

HkTrampoline<void, PlayerHitPointData*> NoDamageHook = hk::hook::trampoline([](PlayerHitPointData* hitPointData) -> void {
    if (!SettingsMgr::instance()->getSettings()->mIsEnableNoDamage) NoDamageHook.orig(hitPointData);
});

HkTrampoline<bool, GameDataHolderAccessor> warpTextHook = hk::hook::trampoline([](GameDataHolderAccessor accessor) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableRefreshWarpText ? false : warpTextHook.orig(accessor);
});

HkTrampoline<bool, GameDataHolderAccessor, al::ActorInitInfo*> refreshPurpsHook =
    hk::hook::trampoline([](GameDataHolderAccessor accessor, al::ActorInitInfo* actorInitInfo) -> bool {
        return SettingsMgr::instance()->getSettings()->mIsEnableRefreshPurps ? false : refreshPurpsHook.orig(accessor, actorInitInfo);
    });

HkTrampoline<void, DoorAreaChange*, al::ActorInitInfo> doorRefreshHook =
    hk::hook::trampoline([](DoorAreaChange* doorAreaChange, al::ActorInitInfo info) -> void {
        doorRefreshHook.orig(doorAreaChange, info);

        if (SettingsMgr::instance()->getSettings()->mIsEnableDoorRefresh) doorAreaChange->switchCloseAgain();
    });

HkTrampoline<void, ShineChip*, al::ActorInitInfo> shardRefreshHook = hk::hook::trampoline([](ShineChip* shineChip, al::ActorInitInfo info) -> void {
    shardRefreshHook.orig(shineChip, info);
    if (SettingsMgr::instance()->getSettings()->mIsEnableShardRefresh) {
        // al::setNerve((al::IUseNerve*)shineChip, (al::Nerve*)(hk::ro::getMainModule()->range().start() + 0x1cbeaf8));
        al::setNerve((al::IUseNerve*)shineChip, (al::Nerve*)(hk::ro::getMainModule()->range().start() + 0x1cbeaf0));
    }
});

HkTrampoline<void, CheckpointFlag*> checkpointFlagHook = hk::hook::trampoline([](CheckpointFlag* checkpointFlag) -> void {
    if (!SettingsMgr::instance()->getSettings()->mIsEnableNoCheckpointTouch) checkpointFlagHook.orig(checkpointFlag);
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

struct MofumofuPatternEntry {
    const char* typeStr;
    int target;
    bool reverse;
};

constexpr static const MofumofuPatternEntry mPatternEntries[22] = {
    { "Ghost", 0, false }, { "Nose", 0, true },     { "C", 1, false },       { "W", 1, true },       { "J", 2, false },        { "Medal", 2, true },
    { "Plane", 3, false }, { "5", 3, true },        { "Hangman", 4, false }, { "Spanish", 4, true }, { "Siblings", 5, false }, { "Snake", 5, true },
    { "8", 6, false },     { "Mushroom", 6, true }, { "Z", 7, false },       { "Tetris", 7, true },  { "Ear", 8, false },      { "Bomb", 8, true },
    { "Bird", 9, false },  { "L", 9, true },        { "O", 10, false },      { "Star", 10, true }
};

bool isPatternReverse() {
    bool b = al::isHalfProbability();
    if (SettingsMgr::instance()->getSettings()->mWigglerPattern != 0)
        b = mPatternEntries[SettingsMgr::instance()->getSettings()->mWigglerPattern - 1].reverse == 1;
    return b;
}

int getPatternTarget(int a) {
    int r = al::getRandom(a);
    if (SettingsMgr::instance()->getSettings()->mWigglerPattern == 0) return r;
    return mPatternEntries[SettingsMgr::instance()->getSettings()->mWigglerPattern - 1].target;
}

void installWigglerHooks() {
    ptr addr = sail::lookupSymbolFromDb<>("WigglerPatternReverse");
    ptr offset = addr - ro::getMainModule()->range().start();
    hk::hook::writeBranchLink(ro::getMainModule(), offset, (void*)isPatternReverse);

    addr = sail::lookupSymbolFromDb<>("WigglerPatternTarget");
    offset = addr - ro::getMainModule()->range().start();
    hk::hook::writeBranchLink(ro::getMainModule(), offset, (void*)getPatternTarget);
}

HkTrampoline<bool, GameDataHolderAccessor, int> allCheckpointsHook = hk::hook::trampoline([](GameDataHolderAccessor acc, int checkpointIdx) -> bool {
    if (SettingsMgr::instance()->getSettings()->mIsEnableAllCheckpoints) {
        auto checkpointTrans = GameDataFunction::getCheckpointTransInWorld(acc, checkpointIdx);
        if (checkpointTrans.x == 0 && checkpointTrans.y == 0 && checkpointTrans.z == 0) {
            return allCheckpointsHook.orig(acc, checkpointIdx);
        }
        return true;
    }
    return allCheckpointsHook.orig(acc, checkpointIdx);
});

HkTrampoline<void, PlayerActorHakoniwa*> noclipHook = hk::hook::trampoline([](PlayerActorHakoniwa* player) -> void {
    static bool wasNoclipOn = false;
    bool isNoclip = Menu::instance()->mIsEnableNoclip;
    if (!isNoclip && wasNoclipOn) player->endDemoPuppetable();
    wasNoclipOn = isNoclip;

    if (!isNoclip) {
        noclipHook.orig(player);
        return;
    }

    if (isNoclip) {
        static float speed = 20.0f;
        static float speedMax = 250.0f;
        static float vspeed = 10.0f;
        static float speedGain = 0.0f;

        sead::Vector3f* playerPos = al::getTransPtr(player);
        sead::Vector3f cameraPos = al::getCameraPos(player, 0);
        sead::Vector2f leftStick = { InputHelper::getLeftStickX(), InputHelper::getLeftStickY() };

        player->startDemoPuppetable();

        float d = sqrt((playerPos->x - cameraPos.x) * (playerPos->x - cameraPos.x) + ((playerPos->z - cameraPos.z) * (playerPos->z - cameraPos.z)));
        float vx = ((speed + speedGain) / d) * (playerPos->x - cameraPos.x);
        float vz = ((speed + speedGain) / d) * (playerPos->z - cameraPos.z);

        playerPos->x -= leftStick.x * vz;
        playerPos->z += leftStick.x * vx;

        playerPos->x += leftStick.y * vx;
        playerPos->z += leftStick.y * vz;

        if (InputHelper::isHoldX() || InputHelper::isHoldY()) speedGain += 0.5f;
        if (InputHelper::isHoldA() || InputHelper::isHoldB()) speedGain -= 0.5f;
        if (speedGain <= 0.0f) speedGain = 0.0f;
        if (speedGain >= speedMax) speedGain = speedMax;

        if (InputHelper::isHoldZL()) playerPos->y -= (vspeed + speedGain / 3);
        if (InputHelper::isHoldZR()) playerPos->y += (vspeed + speedGain / 3);
    }

    noclipHook.orig(player);
});

static void setMapTargetUpdateNullNerve(al::IUseNerve* user, const al::Nerve* nerve) {
    StageSceneStateStageMap* map;
    __asm("mov %0, x19" : "=r"(map));
    void* mapThingPtr;
    __asm("mov %0, x23" : "=r"(mapThingPtr));

    if (mapThingPtr != nullptr) Menu::instance()->setLatestMapTarget(mapThingPtr);

    // al::setNerve(user, nerve);
    user->getNerveKeeper()->setNerve(nerve);
}

HkTrampoline<int, al::LiveActor*, al::PlacementId*> seedRefreshHook = hk::hook::trampoline([](al::LiveActor* actor, al::PlacementId* id) -> int {
    return SettingsMgr::instance()->getSettings()->mIsEnableFlowerPotRefresh ? 0 : seedRefreshHook.orig(actor, id);
});

HkTrampoline<bool, al::LiveActor*, al::PlacementId*> seedRefreshHook2 = hk::hook::trampoline([](al::LiveActor* actor, al::PlacementId* id) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableFlowerPotRefresh ? false : seedRefreshHook2.orig(actor, id);
});

HkTrampoline<bool, al::LiveActor*, char*> hintPhotoHook = hk::hook::trampoline([](al::LiveActor* actor, char* name) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableHintPhotoSpawn ? true : hintPhotoHook.orig(actor, name);
});

constexpr static const char* worldTravelingStatus[] = { "Init", "CityWorld0", "WaterfallWorld0", "LavaWorld0", "MoonWorld0", "PeachCastleWorld0",
                                                        "Last", "LastAfter" };

HkTrampoline<const char*, WorldTravelingNpc*> worldTravelingHook = hk::hook::trampoline([](WorldTravelingNpc* actor) -> const char* {
    // Menu::instance()->mWorldTravelingStatus = worldTravelingHook.orig(actor);
    if (SettingsMgr::instance()->getSettings()->mWorldTravelingStatus != 0) {
        return worldTravelingStatus[SettingsMgr::instance()->getSettings()->mWorldTravelingStatus - 1];
    }
    return worldTravelingHook.orig(actor);
});
HkTrampoline<bool, WorldTravelingNpc*> worldTravelingHook2 = hk::hook::trampoline([](WorldTravelingNpc* actor) -> bool {
    // Menu::instance()->mWorldTravelingStatus = worldTravelingHook.orig(actor);
    if (SettingsMgr::instance()->getSettings()->mWorldTravelingStatus != 0) {
        return false;
    }
    return worldTravelingHook.orig(actor);
});
HkTrampoline<bool, al::LiveActor*> toadRefreshHook = hk::hook::trampoline([](al::LiveActor* actor) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableRefreshNpc ? false : toadRefreshHook.orig(actor);
});

HkTrampoline<void, al::LiveActor*, al::PlacementId*, int*> coinStackRefreshHook =
    hk::hook::trampoline([](al::LiveActor* actor, al::PlacementId* id, int* i) -> void {
        if (SettingsMgr::instance()->getSettings()->mIsEnableDisableCoinStackSave) {
            return;
        } else {
            coinStackRefreshHook.orig(actor, id, i);
        }
    });

HkTrampoline<bool, GameProgressData*, int> kingdomEnterHook = hk::hook::trampoline([](GameProgressData* data, int i) -> bool {
    return SettingsMgr::instance()->getSettings()->mIsEnableRefreshKingdomEnter ? false : kingdomEnterHook.orig(data, i);
});

void SettingsHooks::installSettingsHooks() {
    installDemoHooks();
    installWigglerHooks();

    ptr addr = sail::lookupSymbolFromDb<>("UpdateMapTarget");
    ptr offset = addr - ro::getMainModule()->range().start();
    hk::hook::writeBranchLink(ro::getMainModule(), offset, (void*)setMapTargetUpdateNullNerve);

    addr = sail::lookupSymbolFromDb<>("MoonRefreshText");
    offset = addr - ro::getMainModule()->range().start();
    ro::getMainModule()->writeRo(offset, "BTT Studio", strlen("BTT Studio") + 1);

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
    doorRefreshHook.installAtSym<"_ZN14DoorAreaChange4initERKN2al13ActorInitInfoE">();
    seedRefreshHook.installAtSym<"_ZN2rs17getGrowFlowerTimeEPKN2al9LiveActorEPKNS0_11PlacementIdE">();
    seedRefreshHook2.installAtSym<"_ZN2rs20isUsedGrowFlowerSeedEPKN2al9LiveActorEPKNS0_11PlacementIdE">();
    checkpointFlagHook.installAtSym<"_ZN2rs31setTouchCheckpointFlagToWatcherEP14CheckpointFlag">();
    cloudSkipHook.installAtSym<"_ZNK10StageScene16isDefeatKoopaLv1Ev">();
    allCheckpointsHook.installAtSym<"_ZN16GameDataFunction22isGotCheckpointInWorldE22GameDataHolderAccessori">();
    noclipHook.installAtSym<"_ZN19PlayerActorHakoniwa8movementEv">();
    hintPhotoHook.installAtSym<"_ZN2rs19checkSavedHintPhotoEPKN2al9LiveActorEPKc">();
    worldTravelingHook.installAtSym<"_ZN16GameDataFunction23getWorldTravelingStatusEPK17WorldTravelingNpc">();
    worldTravelingHook2.installAtSym<"_ZN16GameDataFunction27isFirstWorldTravelingStatusEPK17WorldTravelingNpc">();
    toadRefreshHook.installAtSym<"_ZN2rs34isOnFlagKinopioBrigadeNpcFirstTalkEPKN2al9LiveActorE">();
    // shardRefreshHook.installAtSym<"_ZN9ShineChip4initERKN2al13ActorInitInfoE">();
    coinStackRefreshHook.installAtSym<"_ZN2rs13saveCoinStackEPKN2al9LiveActorEPKNS0_11PlacementIdEi">();
    kingdomEnterHook.installAtSym<"_ZNK16GameProgressData16isAlreadyGoWorldEi">();
}
