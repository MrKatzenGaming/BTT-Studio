#include "settings/TimerHooks.h"

#include <hk/hook/Trampoline.h>

#include <al/Library/LiveActor/ActorClippingFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Nerve/NerveKeeper.h>
#include <al/Library/Nerve/NerveUtil.h>

#include <custom/game/Scene/StageScene.h>

#include "menu/Timer.h"

using namespace hk;
using namespace btt;

class ShineCounter : public al::LiveActor {};

static void shineGrabHooks(al::LiveActor* shine) {
    al::invalidateClipping(shine);
    Timer::sInstance->event(TimerHookType::ShineGrab);
}

HkTrampoline shineTickHook = [](TrampolineStatic(), ShineCounter* thisPtr) -> void {
    if (thisPtr->getNerveKeeper()->getCurrentStep() == 1) {
        Timer::sInstance->event(TimerHookType::ShineTick);
    }
    orig(thisPtr);
};
HkTrampoline shineGrabHook = [](TrampolineStatic(), StageScene* thisPtr) -> void {
    if (thisPtr->getNerveKeeper()->getCurrentStep() == 1) {
        Timer::sInstance->event(TimerHookType::ShineGrab);
    }
    orig(thisPtr);
};

void TimerHooks::installTimerHooks() {
    shineTickHook.installAtSym<"_ZN12ShineCounter16exeShineCountAddEv">();
    shineGrabHook.installAtSym<"_ZN10StageScene15exeDemoShineGetEv">();
}
