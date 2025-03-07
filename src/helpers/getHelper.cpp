#include "getHelper.h"

#include "al/Library/LiveActor/LiveActorKit.h"
#include "al/Library/Player/PlayerHolder.h"
#include "al/Library/Scene/Scene.h"

namespace helpers {

al::LiveActor* tryGetPlayerHak(al::Sequence* sequence) {

    HakoniwaSequence* HakSequence = static_cast<HakoniwaSequence*>(sequence);
    if (HakSequence == nullptr)
        return nullptr;
    al::Scene* scene = HakSequence->mCurrentScene;
    al::LiveActor* player = nullptr;
    if (scene && scene->mLiveActorKit && scene->mLiveActorKit->mPlayerHolder)
        player = scene->mLiveActorKit->mPlayerHolder->tryGetPlayer(0);

    return player;
}

}  // namespace helpers