#include "hk/sail/detail.h"
#include "hk/util/Math.h"

#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/Memory/HeapUtil.h"
#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"

#include "game/Player/PlayerFunction.h"
#include "game/Player/PlayerHackKeeper.h"
#include "game/System/GameDataFunction.h"

#include <cxxabi.h>
#include <typeinfo>

#include "helpers/getHelper.h"
#include "helpers/saveFileHelper.h"
#include "imgui.h"
#include "Menu.h"
#include "stage_warp.h"

using namespace btt;

void Menu::drawTeleportCat() {
    if (ImGui::CollapsingHeader("Teleport")) {
        ImGui::Indent();
        ImGui::PushItemWidth(200);
        ImGui::InputInt("Teleport Index", &tpIndex);
        if (tpIndex < 0) tpIndex = hk::util::arraySize(tpStates) - 1;
        if (tpIndex >= hk::util::arraySize(tpStates)) tpIndex = 0;
        ImGui::PopItemWidth();

        if (ImGui::Button("Save")) saveTeleport(tpStates[tpIndex]);
        ImGui::SameLine();
        if (ImGui::Button("Load")) loadTeleport(tpStates[tpIndex]);
        ImGui::SameLine();
        ImGui::PushID("TpHotkeys");
        ImGui::Checkbox("Hotkeys", &set->getSettings()->mIsEnableTpHotkeys);
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Checkbox("Saved", &tpStates[tpIndex].saved);
        ImGui::Text("Stage: %s", tpStates[tpIndex].stageName);
        ImGui::EndDisabled();

        if (ImGui::Button("Save To File")) {
            SaveFileHelper::instance()->saveTeleport(tpStates, hk::util::arraySize(tpStates));
        }

        ImGui::SameLine();
        if (ImGui::Button("Load From File")) {
            SaveFileHelper::instance()->loadTeleport(tpStates, hk::util::arraySize(tpStates), al::getStationedHeap());
        }
        ImGui::Unindent();
    }
}

void Menu::saveTeleport(TpState& state) {
    if (!stageScene || !playerHak) return;

    state.saved = true;
    state.pos = al::getTrans(playerHak);
    state.quat = al::getQuat(playerHak);
    strcpy(state.stageName, getEnglishName(GameDataFunction::getCurrentStageName(GameDataHolderAccessor(stageScene))));
}

void Menu::loadTeleport(TpState& state) {
    if (!stageScene || !playerHak) return;
    al::LiveActor* hack = playerHak->mHackKeeper->mCurrentHackActor;

    if (hack) {
        al::setTrans(hack, state.pos);
        al::updatePoseQuat(hack, state.quat);
        al::setVelocityZero(hack);
        menuTimer = 0;
        return;
    }

    if (set->getSettings()->mIsEnableDisableTpPuppet && helpers::isGetShineState(stageScene)) {
        al::setTrans(playerHak, state.pos);
        al::updatePoseQuat(playerHak, state.quat);
        menuTimer = 0;
    } else {
        playerHak->startDemoPuppetable();
        al::setTrans(playerHak, state.pos);
        al::updatePoseQuat(playerHak, state.quat);
        playerHak->endDemoPuppetable();
        menuTimer = 0;
    }
}

void Menu::drawPageMisc() {
    if (ImGui::CollapsingHeader("Misc")) {
        ImGui::Indent();
        drawTeleportCat();
        ImGui::PushItemWidth(200);
        ImGui::Combo("Wiggler Pattern", &set->getSettings()->mWigglerPattern, WigglerPatterns, IM_ARRAYSIZE(WigglerPatterns));
        ImGui::PopItemWidth();

        if (ImGui::Button("Kill Mario")) {
            if (playerHak) GameDataFunction::killPlayer(GameDataHolderWriter(playerHak));
        }
        ImGui::SameLine();
        if (ImGui::Button("Damage Mario")) {
            if (playerHak) {
                bool tmpDamage = SettingsMgr::instance()->getSettings()->mIsEnableNoDamage;
                SettingsMgr::instance()->getSettings()->mIsEnableNoDamage = false;
                GameDataFunction::damagePlayer(GameDataHolderWriter(playerHak));
                SettingsMgr::instance()->getSettings()->mIsEnableNoDamage = tmpDamage;
            }
        }
        if (ImGui::Button("Life Up Heart")) {
            if (playerHak) GameDataFunction::getLifeMaxUpItem(playerHak);
        }
        ImGui::SameLine();
        if (ImGui::Button("Heal Mario")) {
            if (playerHak) GameDataFunction::recoveryPlayer(playerHak);
        }
        if (ImGui::Button("Add 1000 coins")) {
            if (stageScene) GameDataFunction::addCoin(GameDataHolderWriter(stageScene), 1000);
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove 1000 coins")) {
            if (stageScene) {
                int coinNum = GameDataFunction::getCoinNum(GameDataHolderAccessor(stageScene));
                if (coinNum < 1000)
                    GameDataFunction::addCoin(GameDataHolderWriter(stageScene), -coinNum);
                else
                    GameDataFunction::addCoin(GameDataHolderWriter(stageScene), -1000);
            }
        }
        // if (ImGui::Button("Remove Cappy")) {
        //     if (playerHak) GameDataFunction::disableCapByPlacement((al::LiveActor*)playerHak->mHackCap);
        // }
        if (ImGui::Button("Kill Scene")) {
            if (stageScene) {
                al::NerveKeeper* sceneNerveKeeper = stageScene->getNerveKeeper();
                char* stateName = nullptr;
                char* stateNrvName = nullptr;
                char* stateNrvNameShort = nullptr;
                char* stateNameShort = nullptr;
                bool cmpNrv = false;
                bool cmpState = false;

                if (sceneNerveKeeper->mStateCtrl) {
                    al::NerveStateCtrl::State* state = sceneNerveKeeper->mStateCtrl->mCurrentState;

                    if (state) {
                        const al::Nerve* stateNerve = state->state->getNerveKeeper()->getCurrentNerve();
                        int status;
                        stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
                        stateNameShort = stateName + strlen("StageSceneState");

                        if (stateName) {
                            stateNrvName = abi::__cxa_demangle(typeid(*stateNerve).name(), nullptr, nullptr, &status);

                            if (stateNrvName) {
                                auto prefixLen2 = stateNrvName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
                                stateNrvNameShort = stateNrvName + prefixLen2 + strlen(stateName) + strlen("nrv");

                                cmpNrv = strcmp(stateNrvNameShort, "SkipDemo") == 0 || strcmp(stateNrvNameShort, "Skip") == 0;
                                cmpState = strcmp(stateNameShort, "PauseMenu") == 0 || strcmp(stateNameShort, "SnapShot") == 0;
                            }
                        }
                    }
                    if (stateNrvName) free(stateNrvName);
                    if (stateName) free(stateName);
                    if (!(cmpNrv || cmpState || PlayerFunction::isPlayerDeadStatus(playerHak))) stageScene->kill();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Previous Scene")) {
            if (stageScene) GameDataHolderAccessor(stageScene)->returnPrevStage();
        }
        if (ImGui::Button("Warp to Last Checkpoint")) {
            if (stageScene) {
                if (mLastMapTarget) {
                    stageScene->mStateCollection->mStateStageMap->mMapThing = mLastMapTarget;
                    ptr addr = hk::sail::lookupSymbolFromDb<>("StageSceneNrvWarpToCheckpoint");
                    stageScene->getNerveKeeper()->setNerve((al::Nerve*)addr);
                }
            }
        }
        ImGui::Checkbox("Noclip", &mIsEnableNoclip);
        ImGui::PushItemWidth(200);
        ImGui::Combo("Moon Refresh Text", &set->getSettings()->mMoonRefreshText, MoonRefreshTexts, IM_ARRAYSIZE(MoonRefreshTexts));
        ImGui::Combo("Menu Corner", &set->getSettings()->mMenuCorner, Corners, IM_ARRAYSIZE(Corners));
        ImGui::PopItemWidth();
        ImGui::Unindent();
    }
}
