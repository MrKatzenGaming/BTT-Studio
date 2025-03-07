#include "getHelper.h"

#include "al/Library/LiveActor/LiveActorKit.h"
#include "al/Library/Player/PlayerHolder.h"
#include "game/System/GameSystem.h"
#include "al/Library/Base/StringUtil.h"


namespace helpers {

al::LiveActor* tryGetPlayer(al::Sequence* sequence) {

    HakoniwaSequence* HakSequence = static_cast<HakoniwaSequence*>(sequence);
    if (HakSequence == nullptr)
        return nullptr;
    al::Scene* scene = HakSequence->mCurrentScene;
    al::LiveActor* player = nullptr;
    if (scene && scene->mLiveActorKit && scene->mLiveActorKit->mPlayerHolder)
        player = scene->mLiveActorKit->mPlayerHolder->tryGetPlayer(0);

    return player;
}

al::LiveActor* tryGetPlayer() {
    return tryGetPlayer(GameSystemFunction::getGameSystem()->mSequence);
}

al::Scene *tryGetScene(al::Sequence *sequence) {
    HakoniwaSequence* HakSequence = static_cast<HakoniwaSequence*>(sequence);
    if (HakSequence == nullptr)
        return nullptr;
    return HakSequence->mCurrentScene;
}

al::Scene *tryGetScene() {
    return tryGetScene(GameSystemFunction::getGameSystem()->mSequence);
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

}  // namespace helpers