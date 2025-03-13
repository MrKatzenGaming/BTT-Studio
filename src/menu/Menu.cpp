#include "Menu.h"

#include "InputHelper.h"
#include "helpers/getHelper.h"

#include <cstdio>
#include <heap/seadHeapMgr.h>
#include <nn/oe.h>
#include "al/Library/Camera/CameraUtil.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "game/Scene/StageScene.h"
#include "game/System/GameDataFunction.h"
#include "game/System/GameSystem.h"
#include "hk/util/Math.h"
#include "settings/SettingsMgr.h"
#include "stage_warp.h"

#include "imgui.h"

#include "sead/filedevice/seadFileDeviceMgr.h"
#include "nn/fs.h"

namespace btt {

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

void Menu::draw() {
    HakoniwaSequence* gameSeq = (HakoniwaSequence*)GameSystemFunction::getGameSystem()->mSequence;
    PlayerActorBase* player = helpers::tryGetPlayerActor();
    StageScene* stageScene = helpers::tryGetStageScene();
    SettingsMgr* set = SettingsMgr::instance();

    if (InputHelper::isInputToggled()) {
        drawInputDisabled();
    }

    ImGui::Begin("BTT Studio", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(mWindowSize, ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

    ImGui::Text("Toggle Menu: L-Stick");
    ImGui::Text("Toggle Input: L + DPad-Left");
    ImGui::Separator();

    char fmt[17] = "Toggle Mouse OFF";
    snprintf(fmt, 17, "Toggle Mouse %s", InputHelper::isDisableMouse() ? "ON" : "OFF");

    if (ImGui::Button(fmt)) {
        InputHelper::setDisableMouse(!InputHelper::isDisableMouse());
    }

    if (stageScene) {
        if (ImGui::Button("Kill Scene")) stageScene->kill();
    }

    if (player) {
        if (ImGui::Button("Kill Mario")) {
            GameDataFunction::killPlayer(GameDataHolderWriter(player));
        }
    } 

    if(ImGui::Button("Save Settings")) {
        set->saveSettings();
    }
    if (ImGui::Button("Load Settings")) {
        set->loadSettings();
    }

    drawStageWarpWindow();

    if (ImGui::CollapsingHeader("Options")) {
        ImGui::Checkbox("Moon Refresh", &set->getSettings()->mIsEnableMoonRefresh);
        ImGui::Checkbox("Always Manually Skip Cutscene", &set->getSettings()->mIsEnableAlwaysManualCutscene);
        ImGui::Checkbox("Always Allow Checkpoints", &set->getSettings()->mIsEnableAlwaysCheckpoints);
        ImGui::Checkbox("Disable Auto Save", &set->getSettings()->mIsEnableDisableAutoSave);
        ImGui::Checkbox("Disable Moon Lock", &set->getSettings()->mIsEnableDisableMoonLock);
        ImGui::Checkbox("No Damage", &set->getSettings()->mIsEnableNoDamage);
        ImGui::Checkbox("Disable Music", &set->getSettings()->mIsEnableDisableMusic);
        ImGui::Checkbox("Refresh Warp Text", &set->getSettings()->mIsEnableRefreshWarpText);
        ImGui::Checkbox("Refresh Kingdom Enter Cutscenes", &set->getSettings()->mIsEnableRefreshKingdomEnter);
    }

    if (ImGui::CollapsingHeader("Misc")) {
        ImGui::Indent();
        drawTeleportCat();

        ImGui::Unindent();
    }

    ImGui::End();
}

void Menu::handleInput() {
    if (InputHelper::isPressStickL()) {
        mIsEnabledMenu = !mIsEnabledMenu;
    }
    if (InputHelper::isPressPadLeft() && mIsEnabledTpHotkeys && !InputHelper::isHoldL() && !InputHelper::isInputToggled()) {
        saveTeleport(tpStates[tpIndex]);
    } else if (InputHelper::isPressPadRight() && mIsEnabledTpHotkeys && !InputHelper::isHoldL() && !InputHelper::isInputToggled()) {
        loadTeleport(tpStates[tpIndex]);
    }
}

void Menu::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.IndentSpacing = 10;

    // Change the color of the title bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    // Change the color of the frame background
    colors[ImGuiCol_FrameBg] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.83f, 0.44f, 0.5f, 1.f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    // Change the color of the button
    colors[ImGuiCol_Button] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.83f, 0.44f, 0.5f, 1.f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    colors[ImGuiCol_Header] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.83f, 0.44f, 0.5f, 1.f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);

    colors[ImGuiCol_NavHighlight] = ImVec4(1, 1, 1, 1);
}

void Menu::drawInputDisabled() {
    ImGui::Begin(
        "Input Disabled", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
    );
    ImGui::SetWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(mWindowSize.x + 10, 0), ImGuiCond_FirstUseEver);
    ImGui::SetWindowFontScale(2);
    ImGui::Text("Input Disabled");
    ImGui::End();
}

void Menu::drawTeleportCat() {
    if (ImGui::CollapsingHeader("Teleport")) {
        ImGui::PushItemWidth(200);
        ImGui::InputInt("Teleport Index", &tpIndex);
        if (tpIndex < 0) tpIndex = hk::util::arraySize(tpStates) - 1;
        if (tpIndex >= hk::util::arraySize(tpStates)) tpIndex = 0;
        ImGui::PopItemWidth();

        if (ImGui::Button("Save")) saveTeleport(tpStates[tpIndex]);
        ImGui::SameLine();
        if (ImGui::Button("Load")) loadTeleport(tpStates[tpIndex]);
        ImGui::SameLine();
        ImGui::Checkbox("Hotkeys", &mIsEnabledTpHotkeys);
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Checkbox("Saved", &tpStates[tpIndex].saved);
        ImGui::EndDisabled();
    }
}

void Menu::saveTeleport(TpState& state) {
    StageScene* stageScene = helpers::tryGetStageScene();
    if (!stageScene) return;
    PlayerActorBase* player = helpers::tryGetPlayerActor(stageScene);
    if (!player) return;
    sead::LookAtCamera cam = al::getLookAtCamera(stageScene, 0);

    state.saved = true;
    state.pos = al::getTrans(player);
    state.quat = al::getQuat(player);
}

void Menu::loadTeleport(TpState& state) {
    StageScene* stageScene = helpers::tryGetStageScene();
    if (!stageScene) return;
    PlayerActorBase* player = helpers::tryGetPlayerActor(stageScene);
    if (!player) return;
    sead::LookAtCamera cam = al::getLookAtCamera(stageScene, 0);

    player->startDemoPuppetable();

    al::setTrans(player, state.pos);
    al::updatePoseQuat(player, state.quat);

    player->endDemoPuppetable();
}
} // namespace btt
