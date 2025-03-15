#include "getHelper.h"

#include <cxxabi.h>
#include <nn/init.h>
#include <typeinfo>
#include "al/Library/Base/StringUtil.h"
#include "al/Library/LiveActor/LiveActorKit.h"
#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"
#include "al/Library/Player/PlayerHolder.h"
#include "al/Library/Player/PlayerUtil.h"
#include "al/Library/Scene/SceneUtil.h"
#include "game/Sequence/ChangeStageInfo.h"
#include "game/System/GameDataFunction.h"
#include "game/System/GameSystem.h"

namespace helpers {

bool isInScene() {
    al::Sequence* mSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (mSequence && al::isEqualString(mSequence->mName.cstr(), "HakoniwaSequence")) {
        auto curScene = mSequence->mCurrentScene;

        return curScene && curScene->mIsAlive;
    }

    return false;
}

bool isInScene(al::Scene* curScene) {
    return curScene && curScene->mIsAlive;
}

bool isInStageScene() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        auto gameSeq = (HakoniwaSequence*)curSequence;
        auto curScene = gameSeq->mCurrentScene;

        return curScene && curScene->mIsAlive && al::isEqualString(curScene->mName.cstr(), "StageScene");
    }

    return false;
}

bool isInStageScene(al::Scene* scene) {
    return scene && scene->mIsAlive && al::isEqualString(scene->mName.cstr(), "StageScene");
}

al::Sequence* tryGetSequence() {
    return GameSystemFunction::getGameSystem()->mSequence;
}

HakoniwaSequence* tryGetHakoniwaSequence() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        return (HakoniwaSequence*)curSequence;
    }

    return nullptr;
}

al::Scene* tryGetScene() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;

    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        auto curScene = curSequence->mCurrentScene;

        if (curScene && curScene->mIsAlive) return curScene;
    }

    return nullptr;
}

al::Scene* tryGetScene(al::Sequence* curSequence) {
    auto curScene = curSequence->mCurrentScene;

    if (curScene && curScene->mIsAlive) return curScene;

    return nullptr;
}

al::Scene* tryGetScene(HakoniwaSequence* curSequence) {
    auto curScene = curSequence->mCurrentScene;

    if (curScene && curScene->mIsAlive) return curScene;

    return nullptr;
}

StageScene* tryGetStageScene() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        auto gameSeq = (HakoniwaSequence*)curSequence;
        auto curScene = gameSeq->mCurrentScene;

        if (curScene && curScene->mIsAlive && al::isEqualString(curScene->mName.cstr(), "StageScene")) return (StageScene*)gameSeq->mCurrentScene;
    }

    return nullptr;
}

StageScene* tryGetStageScene(HakoniwaSequence* curSequence) {
    auto curScene = curSequence->mCurrentScene;

    if (curScene && curScene->mIsAlive && al::isEqualString(curScene->mName.cstr(), "StageScene")) return (StageScene*)curScene;

    return nullptr;
}

GameDataHolder* tryGetGameDataHolder() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        HakoniwaSequence* gameSequence = (HakoniwaSequence*)curSequence;
        return gameSequence->mGameDataHolderAccessor.mData;
    }

    return nullptr;
}

GameDataHolder* tryGetGameDataHolder(HakoniwaSequence* curSequence) {
    return curSequence->mGameDataHolderAccessor.mData;
}

GameDataHolder* tryGetGameDataHolder(StageScene* scene) {
    return scene->mHolder->mData;
}

GameDataHolderAccessor* tryGetGameDataHolderAccess() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        HakoniwaSequence* gameSequence = (HakoniwaSequence*)curSequence;
        return &gameSequence->mGameDataHolderAccessor;
    }

    return nullptr;
}

GameDataHolderAccessor* tryGetGameDataHolderAccess(HakoniwaSequence* curSequence) {
    return &curSequence->mGameDataHolderAccessor;
}

GameDataHolderAccessor* tryGetGameDataHolderAccess(StageScene* scene) {
    return scene->mHolder;
}

PlayerActorBase* tryGetPlayerActor() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        auto gameSeq = (HakoniwaSequence*)curSequence;
        auto curScene = gameSeq->mCurrentScene;

        if (curScene && curScene->mIsAlive) {
            return tryGetPlayerActor(curScene);
        }
    }

    return nullptr;
}

PlayerActorBase* tryGetPlayerActor(HakoniwaSequence* curSequence) {
    auto curScene = curSequence->mCurrentScene;

    if (curScene && curScene->mIsAlive) {
        return tryGetPlayerActor(curScene);
    }

    return nullptr;
}

PlayerActorBase* tryGetPlayerActor(al::Scene* scene) {
    if (!isInStageScene(scene)) return nullptr;
    if (!scene) return nullptr;

    al::PlayerHolder* pHolder = al::getScenePlayerHolder(scene);
    if (!pHolder) return nullptr;

    PlayerActorBase* playerBase = (PlayerActorBase*)al::tryGetPlayerActor(pHolder, 0);
    return playerBase;
}

PlayerActorHakoniwa* tryGetPlayerActorHakoniwa() {
    al::Sequence* curSequence = GameSystemFunction::getGameSystem()->mSequence;
    if (curSequence && al::isEqualString(curSequence->mName.cstr(), "HakoniwaSequence")) {
        auto gameSeq = (HakoniwaSequence*)curSequence;
        auto curScene = gameSeq->mCurrentScene;

        if (curScene) return tryGetPlayerActorHakoniwa(curScene);
    }

    return nullptr;
}

PlayerActorHakoniwa* tryGetPlayerActorHakoniwa(HakoniwaSequence* curSequence) {
    auto curScene = curSequence->mCurrentScene;

    if (curScene) return tryGetPlayerActorHakoniwa(curScene);

    return nullptr;
}

PlayerActorHakoniwa* tryGetPlayerActorHakoniwa(al::Scene* scene) {
    if (!isInStageScene()) return nullptr;

    PlayerActorBase* playerBase = (PlayerActorBase*)rs::getPlayerActor(scene);

    if (al::isEqualString(typeid(*playerBase).name(), typeid(PlayerActorHakoniwa).name())) return (PlayerActorHakoniwa*)playerBase;

    return nullptr;
}

bool tryReloadStage() {
    GameDataHolder* holder = tryGetGameDataHolder();
    if (!holder) return false;
    StageScene* scene = tryGetStageScene();
    if (!scene) return false;

    ChangeStageInfo stageInfo(
        holder, "start", GameDataFunction::getCurrentStageName(*scene->mHolder), false, -1, ChangeStageInfo::SubScenarioType::NO_SUB_SCENARIO
    );
    GameDataFunction::tryChangeNextStage(GameDataHolderWriter(scene), &stageInfo);
    return true;
}

bool isGetShineState(StageScene* stageScene) {
    if (!stageScene) return false;

    size_t demangledSize = 0xff;
    char* stateName = nullptr;
    char* demangledBuf = static_cast<char*>(nn::init::GetAllocator()->Allocate(demangledSize));
    int status;
    bool cmp = false;

    const al::Nerve* stageNerve = stageScene->getNerveKeeper()->getCurrentNerve();
    if (!stageScene->getNerveKeeper()->mStateCtrl) return false;

    al::NerveStateCtrl::State* state = stageScene->getNerveKeeper()->mStateCtrl->findStateInfo(stageNerve);
    if (!state) return false;
    
    const al::Nerve* stateNerve = state->state->getNerveKeeper()->getCurrentNerve();
    stateName = abi::__cxa_demangle(typeid(*state->state).name(), demangledBuf, &demangledSize, &status);
    cmp = strcmp(stateName, "StageSceneStateGetShine") == 0;
    nn::init::GetAllocator()->Free(demangledBuf);

    return cmp;
}

} // namespace helpers
