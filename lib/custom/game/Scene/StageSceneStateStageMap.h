#pragma once

#include "nn/types.h"

class StageSceneStateStageMap {
public:
    u8 _0[0x28];
    void* mMapThing = nullptr;
};

class StageSceneStateCollectionList {
public:
    u8 _0[0x38];
    StageSceneStateStageMap* mStateStageMap = nullptr;
};
