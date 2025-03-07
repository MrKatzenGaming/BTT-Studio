#pragma once

#include "game/Sequence/HakoniwaSequence.h"
#include "al/Library/LiveActor/LiveActor.h"

namespace helpers {
    al::LiveActor* tryGetPlayerHak(al::Sequence* sequence);

} // namespace helpers