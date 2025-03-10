#include "Menu.h"

#include "InputHelper.h"
#include "helpers/getHelper.h"

#include <heap/seadHeapMgr.h>
#include <nn/oe.h>
#include "game/Scene/StageScene.h"
#include "game/System/GameDataFunction.h"
#include "game/System/GameSystem.h"
#include "settings/SettingsMgr.h"
#include "stage_warp.h"

#include "imgui.h"

namespace btt {

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

void Menu::draw() {
    HakoniwaSequence* gameSeq = (HakoniwaSequence*)GameSystemFunction::getGameSystem()->mSequence;
    PlayerActorBase* player = helpers::tryGetPlayerActor();
    StageScene* stageScene = helpers::tryGetStageScene();
    SettingsMgr* settings = SettingsMgr::instance();

    if (InputHelper::isInputToggled()) {
        drawInputDisabled();
    }

    ImGui::Begin("BTT Studio", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(mWindowSize, ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

    ImGui::Text("Toggle Menu: L Stick");
    ImGui::Text("Toggle Input: L + DPad-Left");
    ImGui::Separator();

    if (stageScene) {
        if (ImGui::Button("Kill Scene")) stageScene->kill();
    }

    if (player) {
        if (ImGui::Button("Kill Mario")) {
            GameDataFunction::killPlayer(GameDataHolderWriter(player));
        }
    }

    drawStageWarpWindow();

    if (ImGui::CollapsingHeader("Options")) {
        ImGui::Checkbox("Moon Jump", &settings->mSettings.mIsEnableMoonJump);
        ImGui::Checkbox("Moon Refresh", &settings->mSettings.mIsEnableMoonRefresh);
    }

    ImGui::End();
}

void Menu::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Change the color of the title bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    // Change the color of the frame background
    colors[ImGuiCol_FrameBg] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    // Change the color of the button
    colors[ImGuiCol_Button] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    colors[ImGuiCol_Header] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    colors[ImGuiCol_NavHighlight] = ImVec4(1, 1, 1, 1);
}

void Menu::drawInputDisabled() {
    ImGui::Begin(
        "Input Disabled", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
    );
    ImGui::SetWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(mWindowSize.x + 10, 0), ImGuiCond_FirstUseEver);
    ImGui::SetWindowFontScale(2);
    ImGui::Text("Input Disabled");
    ImGui::End();
}

} // namespace btt
