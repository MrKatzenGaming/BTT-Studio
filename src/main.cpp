#include "InputHelper.h"
#include "Menu.h"
#include "game/System/GameFrameworkNx.h"
#include "hk/hook/Trampoline.h"

#include "game/System/Application.h"
#include "game/System/GameSystem.h"

#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/Memory/HeapUtil.h"

#include "settings/SettingsMgr.h"

#include "imgui.h"
#include "imgui_nvn.h"

using namespace hk;
using namespace btt;

void drawFpsWindow() {
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

    ImGui::Begin("FPSCounter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBackground);

    ImGui::Text("FPS: %2.f\n", roundf(Application::instance()->mGameFramework->calcFps()));

    ImGui::End();
}

void drawMenu() {
    Menu* menu = Menu::instance();
    if (menu)
        menu->draw();
}

HkTrampoline<void, GameSystem*> gameSystemInit = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    sead::Heap* heap = al::getStationedHeap();

    SettingsMgr* settingsMgr = SettingsMgr::createInstance(heap);
    Menu* menu = Menu::createInstance(heap);

    nvnImGui::addDrawFunc(drawMenu);
    nvnImGui::addDrawFunc(drawFpsWindow);

    InputHelper::setDisableMouse(true);

    gameSystemInit.orig(gameSystem);
});

HkTrampoline<void, al::LiveActor*> marioControl = hk::hook::trampoline([](al::LiveActor* player) -> void {
    if (InputHelper::isHoldA() && SettingsMgr::instance()->mSettings.mIsEnableMoonJump) {
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

    nvnImGui::InstallHooks();

}
