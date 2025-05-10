#include "hk/sail/detail.h"
#include "hk/util/Math.h"

#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"

#include "game/Player/PlayerFunction.h"
#include "game/System/GameDataFunction.h"
#include <game/Sequence/ChangeStageInfo.h>

#include <cxxabi.h>
#include <typeinfo>

#include "helpers/InputHelper.h"
#include "imgui.h"
#include "Menu.h"

using namespace btt;

void Menu::drawPageHotkeys() {
    if (ImGui::CollapsingHeader("Hotkeys")) {
        ImGui::Indent();
        ImGui::PushItemWidth(200);
        ImGui::Combo("Kill Scene##Key", &set->mSettings.mKillSceneKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Heal Mario##Key", &set->mSettings.mHealMarioKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Life Up Heart##Key", &set->mSettings.mLifeUpKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Prev Scene##Key", &set->mSettings.mPrevSceneKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Increment Tp Index##Key", &set->mSettings.mIncTpIndexKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Decrement Tp Index##Key", &set->mSettings.mDecTpIndexKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Add 1000 Coins##Key", &set->mSettings.mAddCoinsKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Remove 1000 Coins##Key", &set->mSettings.mDecCoinsKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Next Wiggler Pattern##Key", &set->mSettings.mIncPatternKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Prev Wiggler Pattern##Key", &set->mSettings.mDecPatternKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Warp to last Checkpoint##Key", &set->mSettings.mWarpLastCpKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Start Timer##Key", &set->mSettings.mTimerStartKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("End Timer##Key", &set->mSettings.mTimerEndKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::Combo("Reset Timer##Key", &set->mSettings.mTimerResetKey, Keys, IM_ARRAYSIZE(Keys));
        ImGui::PopItemWidth();
        ImGui::Unindent();
    }
}

bool Menu::isHotkey(int key) {
    bool isAllowInput = !InputHelper::isInputToggled() || !mIsEnabledMenu;
    bool Up = InputHelper::isPressPadUp();
    bool L = InputHelper::isHoldL();
    bool R = InputHelper::isHoldR();
    bool ZL = InputHelper::isHoldZL();
    bool ZR = InputHelper::isHoldZR();
    switch (key) {
    case 1: return Up && !L && !R && !ZL && !ZR && isAllowInput;
    case 2: return Up && L && !R && !ZL && !ZR && isAllowInput;
    case 3: return Up && R && !L && !ZL && !ZR && isAllowInput;
    case 4: return Up && ZL && !L && !R && !ZR && isAllowInput;
    case 5: return Up && ZR && !L && !R && !ZL && isAllowInput;
    case 6: return Up && L && R && !ZL && !ZR && isAllowInput;
    case 7: return Up && L && ZL && !R && !ZR && isAllowInput;
    case 8: return Up && L && ZR && !R && !ZL && isAllowInput;
    case 9: return Up && R && ZL && !L && !ZR && isAllowInput;
    case 10: return Up && R && ZR && !L && !ZL && isAllowInput;
    default: return 0;
    }
}

void Menu::handleHotkeys() {
    bool isAllowTP = set->getSettings()->mIsEnableTpHotkeys && (!InputHelper::isInputToggled() || !mIsEnabledMenu);

    if (InputHelper::isPressPadLeft() && isAllowTP) {
        saveTeleport(tpStates[tpIndex]);
    } else if (InputHelper::isPressPadRight() && isAllowTP) {
        loadTeleport(tpStates[tpIndex]);
    }

    if (isHotkey(set->getSettings()->mKillSceneKey)) {
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
                        if (playerHak) {
                            reloadStagePos = al::getTrans(playerHak);
                            reloadStageQuat = al::getQuat(playerHak);
                        }
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
    if (isHotkey(set->getSettings()->mHealMarioKey)) {
        if (playerHak) GameDataFunction::recoveryPlayer(playerHak);
    }
    if (isHotkey(set->getSettings()->mPrevSceneKey)) {
        if (stageScene) GameDataHolderAccessor(stageScene)->returnPrevStage();
    }
    if (isHotkey(set->getSettings()->mIncTpIndexKey)) {
        tpIndex++;
        if (tpIndex >= hk::util::arraySize(tpStates)) tpIndex = 0;
        setPopupText("Tp Index: %d", tpIndex);
    }
    if (isHotkey(set->getSettings()->mDecTpIndexKey)) {
        tpIndex--;
        if (tpIndex < 0) tpIndex = hk::util::arraySize(tpStates) - 1;
        setPopupText("Tp Index: %d", tpIndex);
    }
    if (isHotkey(set->getSettings()->mAddCoinsKey)) {
        if (stageScene) GameDataFunction::addCoin(GameDataHolderWriter(stageScene), 1000);
    }
    if (isHotkey(set->getSettings()->mDecCoinsKey)) {
        if (stageScene) {
            int coinNum = GameDataFunction::getCoinNum(GameDataHolderAccessor(stageScene));
            if (coinNum < 1000)
                GameDataFunction::addCoin(GameDataHolderWriter(stageScene), -(1000 - coinNum));
            else
                GameDataFunction::addCoin(GameDataHolderWriter(stageScene), -1000);
        }
    }
    if (isHotkey(set->getSettings()->mLifeUpKey)) {
        if (playerHak) GameDataFunction::getLifeMaxUpItem(playerHak);
    }
    if (isHotkey(set->getSettings()->mIncPatternKey)) {
        set->getSettings()->mWigglerPattern++;
        if (set->getSettings()->mWigglerPattern >= hk::util::arraySize(WigglerPatterns)) set->getSettings()->mWigglerPattern = 0;
        setPopupText("Wiggler Pattern: %s", WigglerPatterns[set->getSettings()->mWigglerPattern]);
    }
    if (isHotkey(set->getSettings()->mDecPatternKey)) {
        set->getSettings()->mWigglerPattern--;
        if (set->getSettings()->mWigglerPattern < 0) set->getSettings()->mWigglerPattern = hk::util::arraySize(WigglerPatterns) - 1;
        setPopupText("Wiggler Pattern: %s", WigglerPatterns[set->getSettings()->mWigglerPattern]);
    }
    if (isHotkey(set->getSettings()->mWarpLastCpKey)) {
        if (stageScene) {
            if (mLastMapTarget) {
                stageScene->mStateCollection->mStateStageMap->mMapThing = mLastMapTarget;
                ptr addr = hk::sail::lookupSymbolFromDb<>("StageSceneNrvWarpToCheckpoint");
                stageScene->getNerveKeeper()->setNerve((al::Nerve*)addr);
            }
        }
    }
    if (isHotkey(set->getSettings()->mTimerStartKey)) {
        if (Timer::sInstance) {
            Timer::sInstance->start();
            setPopupText("Timer started");
        }
    }
    if (isHotkey(set->getSettings()->mTimerEndKey)) {
        if (Timer::sInstance) {
            Timer::sInstance->stop();
            setPopupText("Timer stopped");
        }
    }
    if (isHotkey(set->getSettings()->mTimerResetKey)) {
        if (Timer::sInstance) {
            Timer::sInstance->reset();
            setPopupText("Timer reset");
        }
    }
}
