#include "getHelper.h"
#include "hk/gfx/DebugRenderer.h"
#include "hk/hook/Trampoline.h"

#include "agl/common/aglDrawContext.h"

#include "game/System/Application.h"
#include "game/System/GameSystem.h"

#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Controller/InputFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/System/GameSystemInfo.h"
#include "al/Library/Memory/HeapUtil.h"

#include "menu/Menu.h"

using namespace hk::hook;

HkTrampoline<void, al::LiveActor*> marioControl = hk::hook::trampoline([](al::LiveActor* player) -> void {
    if (al::isPadHoldA(-1)) {
        player->getPoseKeeper()->getVelocityPtr()->y = 0;
        al::getTransPtr(player)->y += 20;
    }

    marioControl.orig(player);
});

HkTrampoline<void, GameSystem*> gameSystemInit = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    sead::Heap* heap = al::getStationedHeap();

    btt::Menu* menu = btt::Menu::createInstance(heap);
    menu->init(heap);

    gameSystemInit.orig(gameSystem);
});


HkTrampoline<void, GameSystem*> drawMainHook = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    drawMainHook.orig(gameSystem);

    auto* drawContext = Application::instance()->mDrawSystemInfo->drawContext;

    btt::Menu* menu = btt::Menu::instance();
    menu->draw(drawContext);
    
});

// HkTrampoline<bool, al::IUseSceneObjHolder*> isTriggerSnapShotModeHook = hk::hook::trampoline([](al::IUseSceneObjHolder* holder) -> bool {
//     return menu::instance()->mIsEnabledInput ? 0 : isTriggerSnapShotModeHook.orig(holder);
// });

// HkTrampoline<bool, al::IUseSceneObjHolder*> isTriggerAmiiboModeHook = hk::hook::trampoline([](al::IUseSceneObjHolder* holder) -> bool {
//     return menu::instance()->mIsEnabledInput ? 0 : isTriggerAmiiboModeHook.orig(holder);
// });

extern "C" void hkMain() {
    marioControl.installAtSym<"_ZN19PlayerActorHakoniwa7controlEv">();
    gameSystemInit.installAtSym<"_ZN10GameSystem4initEv">();
    drawMainHook.installAtSym<"_ZN10GameSystem8drawMainEv">();

    // isTriggerSnapShotModeHook.installAtSym<"_ZN2rs21isTriggerSnapShotModeEPKN2al18IUseSceneObjHolderE">();
    // isTriggerAmiiboModeHook.installAtSym<"_ZN2rs19isTriggerAmiiboModeEPKN2al18IUseSceneObjHolderE">();

    hk::gfx::DebugRenderer::instance()->installHooks();
}
