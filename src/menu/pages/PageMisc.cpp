#include "hk/sail/detail.h"
#include "hk/util/Math.h"

#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/Memory/HeapUtil.h"
#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"

#include "game/Player/PlayerFunction.h"
#include "game/Player/PlayerHackKeeper.h"
#include "game/System/GameDataFile.h"
#include "game/System/GameDataFunction.h"
#include <game/Sequence/ChangeStageInfo.h>

#include <cxxabi.h>
#include <typeinfo>

#include "helpers/getHelper.h"
#include "helpers/saveFileHelper.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Menu.h"
#include "stage_warp.h"
#include "Timer.h"

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
        al::offCollide(hack);
        al::setTrans(hack, state.pos);
        al::updatePoseQuat(hack, state.quat);
        al::setVelocityZero(hack);
        al::onCollide(hack);
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
    if (ImGui::CollapsingHeader("Misc##page")) {
        ImGui::Indent();
        drawTeleportCat();
        if (ImGui::CollapsingHeader("Outfit")) {
            ImGui::Indent();
            ImGui::Combo("Body", &mSelectedBody, bodyNames, IM_ARRAYSIZE(bodyNames));
            ImGui::Combo("Cap", &mSelectedCap, capNames, IM_ARRAYSIZE(capNames));
            if (ImGui::Button("Set Outfit")) {
                if (gameSeq->mGameDataHolderAccessor.mData && stageScene) {
                    GameDataFunction::wearCap(gameSeq->mGameDataHolderAccessor.mData, capNames[mSelectedCap]);
                    GameDataFunction::wearCostume(gameSeq->mGameDataHolderAccessor.mData, bodyNames[mSelectedBody]);
                    stageScene->kill();
                }
            }
            ImGui::Unindent();
        }
        if (ImGui::CollapsingHeader("Segment Timer")) {
            ImGui::Indent();
            ImGui::Checkbox("Enable", &set->getSettings()->mIsEnableSegmentTimer);
            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                btt::Timer::sInstance->start();
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                btt::Timer::sInstance->stop();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                btt::Timer::sInstance->reset();
            }
            static int startType = 0;
            ImGui::Combo("Start Type", &set->getSettings()->mTimerStartType, TimerHookTypes, IM_ARRAYSIZE(TimerHookTypes));
            ImGui::Combo("End Type", &set->getSettings()->mTimerEndType, TimerHookTypes, IM_ARRAYSIZE(TimerHookTypes));
            static int posX = set->getSettings()->mTimerPos.x;
            static int posY = set->getSettings()->mTimerPos.y;
            ImGui::PushItemWidth(200);
            if (ImGui::InputInt("Timer X", &posX, 5)) set->getSettings()->mTimerPos.x = posX;
            if (ImGui::InputInt("Timer Y", &posY, 5)) set->getSettings()->mTimerPos.y = posY;
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }
        ImGui::PushItemWidth(200);
        ImGui::Combo("Wiggler Pattern", &set->getSettings()->mWigglerPattern, WigglerPatterns, IM_ARRAYSIZE(WigglerPatterns));
        ImGui::Combo("Tourist Status", &set->getSettings()->mWorldTravelingStatus, worldTravelingStatus, IM_ARRAYSIZE(worldTravelingStatus));
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
            if (stageScene && playerHak) {
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
                    if (!(cmpNrv || cmpState || PlayerFunction::isPlayerDeadStatus(playerHak))) {
                        if (mIsReloadPos) {
                            reloadPosTimer = 0;
                            reloadStagePos = al::getTrans(playerHak);
                            reloadStageQuat = al::getQuat(playerHak);
                            ChangeStageInfo info = ChangeStageInfo(
                                gameSeq->mGameDataHolderAccessor, "", GameDataFunction::getCurrentStageName(gameSeq->mGameDataHolderAccessor), false, -1,
                                ChangeStageInfo::SubScenarioType::NO_SUB_SCENARIO
                            );
                            gameSeq->mGameDataHolderAccessor.mData->changeNextStage(&info, 0);
                        } else {
                            stageScene->kill();
                        }
                    }
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
        ImGui::SameLine();
        ImGui::Checkbox("Reload Scene at Pos", &mIsReloadPos);
        if (GImGui->NavId == ImGui::GetID("Reload Scene at Pos")) {
            ImGui::SetTooltip("Doesn't save!!! \nWill break normal reload \n(To fix: re enter stage)");
        }
        ImGui::PushItemWidth(200);
        ImGui::Combo("Moon Refresh Text", &set->getSettings()->mMoonRefreshText, MoonRefreshTexts, IM_ARRAYSIZE(MoonRefreshTexts));
        ImGui::Combo("Menu Corner", &set->getSettings()->mMenuCorner, Corners, IM_ARRAYSIZE(Corners));
        ImGui::PopItemWidth();
        ImGui::Unindent();
    }
}
