#include "hk/util/Math.h"

#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/Memory/HeapUtil.h"

#include "game/Player/PlayerHackKeeper.h"
#include "game/System/GameDataFunction.h"

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
            if (stageScene) stageScene->kill();
        }
        ImGui::SameLine();
        if (ImGui::Button("Previous Scene")) {
            if (stageScene) GameDataHolderAccessor(stageScene)->returnPrevStage();
        }
        ImGui::Checkbox("Noclip", &mIsEnableNoclip);
        ImGui::PushItemWidth(200);
        ImGui::Combo("Moon Refresh Text", &set->getSettings()->mMoonRefreshText, MoonRefreshTexts, IM_ARRAYSIZE(MoonRefreshTexts));
        ImGui::Combo("Menu Corner", &set->getSettings()->mMenuCorner, Corners, IM_ARRAYSIZE(Corners));
        ImGui::PopItemWidth();
        ImGui::Unindent();
    }
}
