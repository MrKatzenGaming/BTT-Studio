#include "helpers/getHelper.h"

#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Base/StringUtil.h>
#include <al/Library/Nerve/NerveKeeper.h>
#include <al/Library/Nerve/NerveStateCtrl.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Player/PlayerUtil.h>
#include <al/Library/Scene/SceneUtil.h>

#include <game/Player/PlayerActorHakoniwa.h>
#include <game/MapObj/ChangeStageInfo.h>
#include <game/System/GameDataFunction.h>
#include <game/System/GameSystem.h>

#include <cstring>
#include <cxxabi.h>
#include <typeinfo>

static int status;

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

        if (curScene && curScene->mIsAlive && al::isEqualString(curScene->mName.cstr(), "StageScene"))
            return (StageScene*)gameSeq->mCurrentScene;
    }

    return nullptr;
}

StageScene* tryGetStageScene(HakoniwaSequence* curSequence) {
    auto curScene = curSequence->mCurrentScene;

    if (curScene && curScene->mIsAlive && al::isEqualString(curScene->mName.cstr(), "StageScene"))
        return (StageScene*)curScene;

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
    return scene->mGameDataHolder;
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

// GameDataHolderAccessor* tryGetGameDataHolderAccess(StageScene* scene) {
//     return scene->mGameDataHolder;
// }

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

    if (al::isEqualString(typeid(*playerBase).name(), typeid(PlayerActorHakoniwa).name()))
        return (PlayerActorHakoniwa*)playerBase;

    return nullptr;
}

bool tryReloadStage() {
    GameDataHolderAccessor* accessor = tryGetGameDataHolderAccess();
    if (!accessor) return false;
    StageScene* scene = tryGetStageScene();
    if (!scene) return false;

    ChangeStageInfo stageInfo(
        accessor->mData, "start", GameDataFunction::getCurrentStageName(*accessor), false, -1,
        ChangeStageInfo::SubScenarioType::NO_SUB_SCENARIO
    );
    GameDataFunction::tryChangeNextStage(GameDataHolderWriter(scene), &stageInfo);
    return true;
}

bool isGetShineState(StageScene* stageScene) {
    if (!stageScene) return false;
    PlayerActorHakoniwa* p = tryGetPlayerActorHakoniwa(stageScene);
    if (!p) return false;

    char* stateNameScene = nullptr;
    char* nrvNameP = nullptr;

    if (!stageScene->getNerveKeeper()->mStateCtrl) return false;
    if (!p->getNerveKeeper()) return false;

    al::NerveStateCtrl::State* stateScene = stageScene->getNerveKeeper()->mStateCtrl->mCurrentState;
    if (!stateScene) return false;
    const al::Nerve* currentNerve = p->getNerveKeeper()->mCurrentNerve;
    if (!currentNerve) return false;

    // stateName = demangle(typeid(*state->state).name()) + strlen("StageSceneState");
    stateNameScene = abi::__cxa_demangle(typeid(*stateScene->state).name(), nullptr, nullptr, &status) +
                     strlen("StageSceneState");
    nrvNameP = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status) +
               strlen("(anonymous namespace)::PlayerActorHakoniwaNrv");

    bool cmp = strcmp(stateNameScene, "GetShine") == 0 && strcmp(nrvNameP, "Demo") == 0;
    free(stateNameScene);
    free(nrvNameP);
    return cmp;

    return false;
}
} // namespace helpers
