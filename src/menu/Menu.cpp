#include "Menu.h"

#include "al/Library/Camera/CameraUtil.h"

#include "game/System/GameDataFunction.h"

#include <cstring>
#include <nn/oe.h>

#include "helpers/getHelper.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "InputHelper.h"
#include "settings/SettingsMgr.h"
#include "stage_warp.h"

namespace btt {

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

static constexpr char windowName[] = "BTT Studio v1.2.1";

void Menu::draw() {
    if (InputHelper::isInputToggled()) {
        drawPopup();
    }

    ImGui::Begin(windowName, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus);
    ImGui::SetWindowSize(mWindowSize, ImGuiCond_FirstUseEver);
    if (!set)
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    else
        ImGui::SetWindowPos(getCornerPos(set->getSettings()->mMenuCorner), ImGuiCond_Always);

    ImGui::Text("Toggle Menu: L-Stick");
    ImGui::Text("Toggle Input: R + ZR + L");
    ImGui::Separator();

    char fmt[17] = "Toggle Mouse OFF";
    snprintf(fmt, 17, "Toggle Mouse %s", InputHelper::isDisableMouse() ? "ON" : "OFF");

    if (ImGui::Button(fmt)) {
        InputHelper::setDisableMouse(!InputHelper::isDisableMouse());
    }

    drawStageWarpWindow();
    drawPageOptions();
    drawPageMisc();
    drawPageInputDisplay();
    drawPageHotkeys();
    drawPageInfo();

    ImGui::End();
}

void Menu::handleAlways() {
    set = SettingsMgr::instance();
    gameSeq = helpers::tryGetHakoniwaSequence();
    stageScene = helpers::tryGetStageScene(gameSeq);
    // BAAAADD
    if (stageScene) {
        if (strcmp(GameDataFunction::getCurrentStageName(gameSeq->mGameDataHolderAccessor), "ClashWorldHomeStage") == 0) noGetPlayer = false;
    }
    if (noGetPlayer) {
        playerHak = nullptr;
    } else {
        playerHak = helpers::tryGetPlayerActorHakoniwa(gameSeq);
    }
    ImGui::NavHighlightActivated(GImGui->NavId);
    ImGui::SetNavCursorVisibleAfterMove();

    menuTimer++;

    static bool wasMenuDisabled = false;
    static bool prevMouseDis = true;

    handleHotkeys();

    if (InputHelper::isPressStickL() && mIsEnabledMenu) {
        prevNavId = GImGui->NavId;
        mIsEnabledMenu = false;
        prevMouseDis = InputHelper::isDisableMouse();
        InputHelper::setDisableMouse(true);
    } else if (InputHelper::isPressStickL() && !mIsEnabledMenu) {
        mIsEnabledMenu = true;
        wasMenuDisabled = true;
        menuTimer = 0;
        InputHelper::setDisableMouse(prevMouseDis);
    }
    if (menuTimer < 5) {
        if (wasMenuDisabled && mIsEnabledMenu) {
            if (prevNavId) ImGui::SetFocusID(prevNavId, ImGui::FindWindowByName(windowName));
        } else if (!mIsPopup) {
            if (playerHak) al::requestCancelCameraInterpole(playerHak, 0);
        }
    } else if (menuTimer == 5 && wasMenuDisabled) {
        wasMenuDisabled = false;
    }
    if (mIsPopup) {
        if (menuTimer > 2 * 60 && menuTimer < 2 * 60 + 5) {
            mIsPopup = false;
            strcpy(popupText, "Input Disabled");
        }
        if (!mIsEnabledMenu) drawPopup();
        if (mIsEnabledMenu && !InputHelper::isInputToggled()) drawPopup();
    }
}

void Menu::drawPopup() {
    ImGui::Begin(
        "Popup", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus
    );
    ImGui::SetWindowSize(ImVec2(0, 0));
    ImGui::SetWindowPos(ImVec2(mWindowSize.x + 10, 0));
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("%s", popupText);
    ImGui::End();
}

const char* Menu::getMoonRefreshText() {
    if (strcmp(MoonRefreshTexts[set->getSettings()->mMoonRefreshText], "<blank>") != 0) {
        return MoonRefreshTexts[set->getSettings()->mMoonRefreshText];
    } else {
        return "";
    }
}

ImVec2 Menu::getCornerPos(int corner) {
    ImVec2 pos = ImVec2(0, 0);
    nn::oe::OperationMode mode = nn::oe::GetOperationMode();
    if (mode == nn::oe::OperationMode_Docked) {
        switch (corner) {
        case 0: pos = ImVec2(0, 0); break;
        case 1: pos = ImVec2(1600 - mWindowSize.x, 0); break;
        case 2: pos = ImVec2(0, 900 - mWindowSize.y); break;
        case 3: pos = ImVec2(1600 - mWindowSize.x, 900 - mWindowSize.y); break;
        }
    } else {
        switch (corner) {
        case 0: pos = ImVec2(0, 0); break;
        case 1: pos = ImVec2(1280 - mWindowSize.x, 0); break;
        case 2: pos = ImVec2(0, 720 - mWindowSize.y); break;
        case 3: pos = ImVec2(1280 - mWindowSize.x, 720 - mWindowSize.y); break;
        }
    }
    return pos;
}

} // namespace btt
