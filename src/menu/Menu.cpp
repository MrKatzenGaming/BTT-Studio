#include "Menu.h"

#include "al/Library/Camera/CameraUtil.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"

#include "game/Scene/StageScene.h"
#include "game/System/GameDataFunction.h"

#include <cstdio>
#include <cstring>
#include <nn/oe.h>

#include "helpers/getHelper.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "InputHelper.h"
#include "settings/SettingsMgr.h"
#include "stage_warp.h"

#ifdef BTTDEBUG
# include "hk/ro/RoUtil.h"

# include "al/Library/Nerve/NerveKeeper.h"
# include "al/Library/Nerve/NerveStateCtrl.h"

# include <sead/heap/seadHeapMgr.h>

# include "game/System/GameDataFile.h"

# include <cxxabi.h>
# include <typeinfo>

# include "logger.h"
#endif

namespace btt {

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

static const char* windowName = "BTT Studio v" VERSION SUBVER;

void Menu::draw() {
    static bool wasInputToggled = false;
    if (InputHelper::isInputToggled()) {
        mIsPopup = true;
        wasInputToggled = true;
    }
    if (wasInputToggled && !InputHelper::isInputToggled()) {
        mIsPopup = false;
        wasInputToggled = false;
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

#ifdef BTTDEBUG
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Debug")) {
        ImGui::Indent();
        if (ImGui::Button("Reconnect logger")) {
            Logger::instance()->connect("192.168.178.41", 8171);
        }
        ImGui::Text("Menu Timer: %lu", menuTimer);
        ImGui::Text("NavId: %u", GImGui->NavId);
        ImGui::Text("Stage: %s", gameSeq ? GameDataFunction::getCurrentStageName(gameSeq->mGameDataHolderAccessor) : NULL);
        ImGui::Text("Scenario: %u", playerHak ? GameDataFunction::getScenarioNo(playerHak) : -1);
        ImGui::Text("Entrance: %s", gameSeq ? gameSeq->mGameDataHolderAccessor.mData->mPlayingFile->mPlayerStartId.cstr() : NULL);
        ImGui::Text("WorldTravelingStatus: %s", mWorldTravelingStatus);

        if (ImGui::CollapsingHeader("Player Sutff")) {
            sead::Quatf quat = sead::Quatf();
            sead::Vector3f pos = sead::Vector3f();
            if (playerHak) {
                pos = al::getTrans(playerHak);
                quat = al::getQuat(playerHak);
            }
            ImGui::Text("Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
            ImGui::Text("Quat: %.2f, %.2f, %.2f, %.2f", quat.x, quat.y, quat.z, quat.w);
            ImGui::Text("ReloadPosTimer: %d", reloadPosTimer);
            ImGui::Text("ReloadPos: %.2f, %.2f, %.2f", reloadStagePos.x, reloadStagePos.y, reloadStagePos.z);
            ImGui::Text("ReloadQuat: %.2f, %.2f, %.2f, %.2f", reloadStageQuat.x, reloadStageQuat.y, reloadStageQuat.z, reloadStageQuat.w);
        }

        drawComplexHeapTreeItem(sead::HeapMgr::instance()->getRootHeap(0));

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Demagnle Stuff")) {
            if (gameSeq) {
                int status;
                uintptr_t NrvAddr = 0;
                al::NerveKeeper* nerveKeeper = gameSeq->getNerveKeeper();
                char* sequenceName = nullptr;
                char* nerveName = nullptr;
                char* stateName = nullptr;
                int prefixLen = 0;

                sequenceName = abi::__cxa_demangle(typeid(*gameSeq).name(), nullptr, nullptr, &status);

                if (nerveKeeper) {
                    const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
                    if (currentNerve) {
                        NrvAddr = (uintptr_t)currentNerve;
                        nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                        prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
                    }
                    if (gameSeq->getNerveKeeper()->mStateCtrl) {
                        al::NerveStateCtrl::State* state = gameSeq->getNerveKeeper()->mStateCtrl->mCurrentState;
                        if (state) stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
                    }
                }

                ImGui::Text("Sequence: %s", sequenceName);
                ImGui::Text("SequenceNrv: %s", nerveName + prefixLen);
                ImGui::Text("SequenceState: %s", stateName);
                ImGui::Text("SequenceNrvAddr: %lx", NrvAddr);
                ImGui::Text("SequenceNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
                ImGui::Separator();

                if (sequenceName) free(sequenceName);
                if (nerveName) free(nerveName);
                if (stateName) free(stateName);
            }
            if (stageScene) {
                int status;
                uintptr_t NrvAddr = 0;
                al::NerveKeeper* nerveKeeper = stageScene->getNerveKeeper();
                char* sceneName = nullptr;
                char* nerveName = nullptr;
                char* stateName = nullptr;
                int prefixLen = 0;

                sceneName = abi::__cxa_demangle(typeid(*stageScene).name(), nullptr, nullptr, &status);

                if (nerveKeeper) {
                    const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
                    if (currentNerve) {
                        NrvAddr = (uintptr_t)currentNerve;
                        nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                        prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
                    }
                    if (stageScene->getNerveKeeper()->mStateCtrl) {
                        al::NerveStateCtrl::State* state = stageScene->getNerveKeeper()->mStateCtrl->mCurrentState;
                        if (state) stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
                    }
                }

                ImGui::Text("StageScene: %s", sceneName);
                ImGui::Text("StageSceneNrv: %s", nerveName + prefixLen);
                ImGui::Text("StageSceneState: %s", stateName);
                ImGui::Text("StageSceneNrvAddr: %lx", NrvAddr);
                ImGui::Text("StageSceneNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
                ImGui::Separator();

                if (sceneName) free(sceneName);
                if (nerveName) free(nerveName);
                if (stateName) free(stateName);
            }
            if (playerHak) {
                int status;
                uintptr_t NrvAddr = 0;
                al::NerveKeeper* nerveKeeper = playerHak->getNerveKeeper();
                char* actorName = nullptr;
                char* nerveName = nullptr;
                char* stateName = nullptr;
                int prefixLen = 0;

                actorName = abi::__cxa_demangle(typeid(*playerHak).name(), nullptr, nullptr, &status);

                if (nerveKeeper) {
                    const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
                    if (currentNerve) {
                        NrvAddr = (uintptr_t)currentNerve;
                        nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                        prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
                    }
                    if (playerHak->getNerveKeeper()->mStateCtrl) {
                        al::NerveStateCtrl::State* state = playerHak->getNerveKeeper()->mStateCtrl->mCurrentState;
                        if (state) stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
                    }
                }

                ImGui::Text("Player: %s", actorName);
                ImGui::Text("PlayerNrv: %s", nerveName + prefixLen);
                ImGui::Text("PlayerState: %s", stateName);
                ImGui::Text("PlayerNrvAddr: %lx", NrvAddr);
                ImGui::Text("PlayerNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
                ImGui::Separator();

                if (actorName) free(actorName);
                if (nerveName) free(nerveName);
                if (stateName) free(stateName);
            }
        }
        ImGui::Unindent();
    }
#endif

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

    if (reloadPosTimer != -1 && playerHak) {
        reloadPosTimer++;
        if (reloadPosTimer >= 11) {
            playerHak->startDemoPuppetable();
            al::setTrans(playerHak, reloadStagePos);
            al::updatePoseQuat(playerHak, reloadStageQuat);
            al::setVelocityZero(playerHak);
            playerHak->endDemoPuppetable();
            menuTimer = 0;
            reloadPosTimer = -1;
        }
    }

    if (InputHelper::isPressStickL() && mIsEnabledMenu) {
        prevNavId = GImGui->NavId;
        mIsEnabledMenu = false;
        prevMouseDis = InputHelper::isDisableMouse();
        InputHelper::setDisableMouse(true);
        if (mIsPopup) {
            mIsPopup = false;
            strcpy(popupText, "Input Disabled");
        }
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
        popupTimer++;
        if (popupTimer > 2 * 60 && popupTimer < 2 * 60 + 5) {
            mIsPopup = false;
            strcpy(popupText, "Input Disabled");
        }
    }
}

void Menu::drawPopup() {
    if (!mIsPopup) return;
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
#ifdef BTTDEBUG
void Menu::drawComplexHeapTreeItem(sead::Heap* heap) {
    ImGui::SetWindowFontScale(1.1f);

    bool hasNoChildren = heap->mChildren.isEmpty();

    ImGui::BeginGroup();

    // Create a unindented tree node, replacing arrow with bullet if no children exist
    bool expanded =
        ImGui::TreeNodeEx(heap->getName().cstr(), ImGuiTreeNodeFlags_NoTreePushOnOpen | (hasNoChildren ? ImGuiTreeNodeFlags_Bullet : ImGuiTreeNodeFlags_None));

    float used = (heap->getSize() - heap->getFreeSize());
    float size = heap->getSize();
    float mbUsed = used / 1000000.f;
    float mbSize = size / 1000000.f;

    float percentUsed = (heap->getSize() - heap->getFreeSize()) / (float(heap->getSize()) / 100);

    char buf[32];
    snprintf(buf, sizeof(buf), "%.3f/%.3f MB", mbUsed, mbSize);
    ImGui::SameLine();
    ImGui::ProgressBar(percentUsed / 100, ImVec2(-FLT_MIN, 0), buf);
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%.0f/%.0f Bytes", used, size);

    if (expanded) {
        for (sead::Heap& childRef : heap->mChildren) {
            sead::Heap* child = &childRef;
            if (child) drawComplexHeapTreeItem(child);
        }
    }
}
#endif

} // namespace btt
