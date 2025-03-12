#include <cstddef>
#include "InputHelper.h"
#include "Menu.h"
#include "game/System/GameFrameworkNx.h"
#include "hk/hook/Trampoline.h"

#include "game/System/Application.h"
#include "game/System/GameSystem.h"

#include "al/Library/Memory/HeapUtil.h"

#include "settings/SettingsHooks.h"
#include "settings/SettingsMgr.h"

#include "imgui.h"
#include "nvnImGui/imgui_nvn.h"

using namespace hk;
using namespace btt;

void drawFpsWindow() {
    if (!Menu::instance()->mIsEnabledMenu) return;
    ImGui::SetNextWindowPos(ImVec2(120.f, -8.f));

    ImGui::Begin(
        "FPSCounter", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoBackground
    );

    ImGui::Text("FPS: %2.f\n", Application::instance()->mGameFramework->calcFps());

    ImGui::End();
}

void drawMenu() {
    Menu* menu = Menu::instance();
    if (menu && menu->mIsEnabledMenu) menu->draw();
    menu->handleInput();
}

HkTrampoline<void, GameSystem*> gameSystemInit = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    sead::Heap* heap = al::getStationedHeap();

    SettingsMgr* settingsMgr = SettingsMgr::createInstance(heap);
    Menu* menu = Menu::createInstance(heap);

    menu->setupStyle();

    nvnImGui::addDrawFunc(drawMenu);
    // nvnImGui::addDrawFunc(drawFpsWindow);

    InputHelper::setDisableMouse(true);

    gameSystemInit.orig(gameSystem);
});

HkTrampoline<void, GameSystem*> drawMainHook = hk::hook::trampoline([](GameSystem* gameSystem) -> void { drawMainHook.orig(gameSystem); });

extern "C" void hkMain() {
    gameSystemInit.installAtSym<"_ZN10GameSystem4initEv">();
    drawMainHook.installAtSym<"_ZN10GameSystem8drawMainEv">();

    SettingsHooks::installSettingsHooks();

    nvnImGui::InstallHooks();
}
