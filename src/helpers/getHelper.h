#pragma once

#include "game/Sequence/HakoniwaSequence.h"
#include "al/Library/Scene/Scene.h"
#include "al/Library/LiveActor/LiveActor.h"

namespace helpers {
    al::LiveActor* tryGetPlayer(al::Sequence* sequence);
    al::LiveActor* tryGetPlayer();
    al::Scene* tryGetScene(al::Sequence* sequence);
    al::Scene* tryGetScene();
    StageScene* tryGetStageScene();

} // namespace helpers