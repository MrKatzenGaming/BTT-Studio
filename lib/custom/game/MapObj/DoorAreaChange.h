#pragma once

#include "Library/LiveActor/LiveActor.h"

class DoorAreaChange : public al::LiveActor {
public:
DoorAreaChange(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void switchCloseAgain();
    
};
