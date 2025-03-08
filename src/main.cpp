#include "InputHelper.h"
#include "getHelper.h"
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
#include "imgui_backend/imgui_impl_nvn.hpp"
#include "sead/heap/seadHeapMgr.h"

#include "settings/SettingsMgr.h"

#include "game/System/GameDataFunction.h"

#include "imgui.h"
#include "imgui_nvn.h"

using namespace hk;
using namespace btt;



// HkTrampoline<bool, al::IUseSceneObjHolder*> isTriggerSnapShotModeHook = hk::hook::trampoline([](al::IUseSceneObjHolder* holder) -> bool {
//     return (Menu::instance()->mIsEnabledInput && Menu::instance()->mIsEnabledMenu) ? 0 : isTriggerSnapShotModeHook.orig(holder);
// });

// HkTrampoline<bool, al::IUseSceneObjHolder*> isTriggerAmiiboModeHook = hk::hook::trampoline([](al::IUseSceneObjHolder* holder) -> bool {
//     return (Menu::instance()->mIsEnabledInput && Menu::instance()->mIsEnabledMenu) ? 0 : isTriggerAmiiboModeHook.orig(holder);
// });

void drawDebugWindow() {
    HakoniwaSequence *gameSeq = (HakoniwaSequence *) GameSystemFunction::getGameSystem()->mSequence;

    if (al::isPadHoldUp(-1) ) {
        helpers::tryGetPlayer()->getPoseKeeper()->getVelocityPtr()->y = 0;
        al::getTransPtr(helpers::tryGetPlayer())->y += 20;
    }

    ImGui::Begin("Game Debug Window");
    ImGui::SetWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    ImGui::Text("Current Sequence Name: %s", gameSeq->mName.cstr());

    static bool showWindow = false;

    if (ImGui::Button("Toggle Demo Window")) {
        showWindow = !showWindow;
    }

    if (showWindow) {
        ImGui::ShowDemoWindow();
    }

    auto curScene = gameSeq->mCurrentScene;

    bool isInGame =
            curScene && curScene->mIsAlive;

    if (isInGame) {
        StageScene *stageScene = (StageScene *)gameSeq->mCurrentScene;
        al::LiveActor *playerBase = helpers::tryGetPlayer();

        if (ImGui::Button("Kill Mario")) {
            GameDataFunction::killPlayer(GameDataHolderWriter(playerBase));
        }
    }

    ImGui::End();
}

void drawFpsWindow() {
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

    ImGui::Begin("FPSCounter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBackground);

    ImGui::Text("FPS: %d\n", 1234234523);

    ImGui::End();
}

HkTrampoline<void, GameSystem*> gameSystemInit = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    sead::Heap* heap = al::getStationedHeap();

    SettingsMgr* settingsMgr = SettingsMgr::createInstance(heap);
    settingsMgr->init(heap);

    nvnImGui::addDrawFunc(drawDebugWindow);
    nvnImGui::addDrawFunc(drawFpsWindow);

    gameSystemInit.orig(gameSystem);
});

HkTrampoline<void, al::LiveActor*> marioControl = hk::hook::trampoline([](al::LiveActor* player) -> void {
    if (al::isPadHoldA(-1) && SettingsMgr::instance()->mSettings.mIsEnableMoonJump) {
        player->getPoseKeeper()->getVelocityPtr()->y = 0;
        al::getTransPtr(player)->y += 20;
    }

    marioControl.orig(player);
});


HkTrampoline<void, GameSystem*> drawMainHook = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    drawMainHook.orig(gameSystem);
    
});

extern "C" void hkMain() {


   


    marioControl.installAtSym<"_ZN19PlayerActorHakoniwa7controlEv">();
    gameSystemInit.installAtSym<"_ZN10GameSystem4initEv">();
    drawMainHook.installAtSym<"_ZN10GameSystem8drawMainEv">();

    // isTriggerSnapShotModeHook.installAtSym<"_ZN2rs21isTriggerSnapShotModeEPKN2al18IUseSceneObjHolderE">();
    // isTriggerAmiiboModeHook.installAtSym<"_ZN2rs19isTriggerAmiiboModeEPKN2al18IUseSceneObjHolderE">();
    nvnImGui::InstallHooks();

}
