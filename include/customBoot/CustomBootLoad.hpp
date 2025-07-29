#pragma once

#include "al/Library/Layout/LayoutActor.h"
#include "al/Library/Layout/LayoutInitInfo.h"
#include "al/Library/Nerve/NerveSetupUtil.h"
#include "custom/al/Library/Play/Layout/WipeSimple.h"

#include "game/Sequence/WorldResourceLoader.h"

namespace customboot {

class CustomBootLoad : public al::LayoutActor {
public:
    CustomBootLoad(WorldResourceLoader* resourceLoader, const al::LayoutInitInfo& initInfo, float autoCloseAfter);

    void exeAppear();
    void exeWait();
    void exeDecrease();
    void exeEnd();

    float mTime = 0.f;
    float mProgression = 0.f;
    float mRotTime = 0.f;

    // Online logo part
    sead::Vector2f mOnlineLogoTrans = sead::Vector2f::zero;
    sead::Vector2f mOnlineLogoTransTarget = sead::Vector2f::zero;
    float mOnlineLogoScale = 0.f;
    float mOnlineLogoScaleTarget = 0.f;
    float mOnlineCreditScale = 1.f;
    float mOnlineCreditScaleTarget = 1.f;

    // Freze tag logo root
    float mFreezeLogoTransX = 1000.f;
    float mFreezeLogoTransXTarget = 1000.f;

    // Borders
    sead::Vector2f mFreezeBorder = { 700.f, 420.f };
    sead::Vector2f mFreezeBorderTarget = { 700.f, 420.f };

    // Backgrounds
    float mFreezeBGTransX = 0.f;
    float mFreezeBGTransXTarget = 0.f;

private:
    float mAutoCloseAfter = 0.f;
    WorldResourceLoader* worldResourceLoader;
    al::WipeSimple* wipe;
    float mDotTimer = 0.0f;
    int mLoadingDotState = 0;
    int mConnectingDotState = 0;

    // Connection state
    bool mHasConnected = false;
    float mConnectionDisplayTimer = 0.0f;

    // Movement after connection
    float mMoveTimer = 0.0f;
    bool mHasStartedMove = false;
};

namespace {

NERVE_IMPL(CustomBootLoad, Appear)
NERVE_IMPL(CustomBootLoad, Wait)
NERVE_IMPL(CustomBootLoad, Decrease)
NERVE_IMPL(CustomBootLoad, End)

NERVES_MAKE_STRUCT(CustomBootLoad, Appear, Wait, Decrease, End)
} // namespace
} // namespace customboot
