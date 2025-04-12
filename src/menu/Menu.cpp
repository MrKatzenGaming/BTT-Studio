#include "Menu.h"

#include "hk/util/Math.h"

#include "al/Library/Camera/CameraUtil.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/Memory/HeapUtil.h"

#include "game/Player/PlayerHackKeeper.h"
#include "game/System/GameDataFunction.h"
#include "game/Util/AchievementUtil.h"

#include <cstdio>
#include <cstring>
#include <nn/oe.h>

#include "helpers/getHelper.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "InputDisplay.h"
#include "InputHelper.h"
#include "saveFileHelper.h"
#include "settings/SettingsMgr.h"
#include "stage_warp.h"

namespace btt {

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

void Menu::draw() {
    if (InputHelper::isInputToggled()) {
        drawPopup();
    }

    ImGui::Begin("BTT Studio", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus);
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

    if (ImGui::CollapsingHeader("Options")) {
        ImGui::Indent();
        ImGui::Checkbox("Moon Refresh", &set->getSettings()->mIsEnableMoonRefresh);
        ImGui::Checkbox("Always Manually Skip Cutscene", &set->getSettings()->mIsEnableAlwaysManualCutscene);
        ImGui::Checkbox("Always Allow Checkpoints", &set->getSettings()->mIsEnableAlwaysCheckpoints);
        ImGui::Checkbox("Disable Auto Save", &set->getSettings()->mIsEnableDisableAutoSave);
        ImGui::Checkbox("Disable Moon Lock", &set->getSettings()->mIsEnableDisableMoonLock);
        ImGui::Checkbox("No Damage", &set->getSettings()->mIsEnableNoDamage);
        ImGui::Checkbox("Disable Music", &set->getSettings()->mIsEnableDisableMusic);
        ImGui::Checkbox("Refresh Warp Text", &set->getSettings()->mIsEnableRefreshWarpText);
        ImGui::Checkbox("Disable Teleport Puppet", &set->getSettings()->mIsEnableDisableTpPuppet);
        // if (GImGui->NavId == ImGui::GetID("Disable Teleport Puppet")) ImGui::SetTooltip("Only in Moon Get Animation");
        ImGui::Checkbox("Refresh Purple Coins", &set->getSettings()->mIsEnableRefreshPurps);
        ImGui::Checkbox("No Checkpoint Touch", &set->getSettings()->mIsEnableNoCheckpointTouch);
        ImGui::Checkbox("Skip Cloud", &set->getSettings()->mIsEnableSkipCloud);
        ImGui::Checkbox("Enable All Checkpoints", &set->getSettings()->mIsEnableAllCheckpoints);

        ImGui::BeginDisabled();
        ImGui::Checkbox("Refresh Doors", &set->getSettings()->mIsEnableDoorRefresh);
        ImGui::Checkbox("Refresh Moon Shards", &set->getSettings()->mIsEnableShardRefresh);
        ImGui::Checkbox("Refresh Kingdom Enter Cutscenes", &set->getSettings()->mIsEnableRefreshKingdomEnter);
        ImGui::Checkbox("Refresh Seeds", &set->getSettings()->mIsEnableFlowerPotRefresh);
        ImGui::EndDisabled();

        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Misc")) {
        ImGui::Indent();
        drawTeleportCat();
        drawMiscCat();
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Input Display")) {
        static int bgColor = u8(set->getSettings()->mInputDisplayBackColor);
        static int ringColor = u8(set->getSettings()->mInputDisplayRingColor);
        static int stickColor = u8(set->getSettings()->mInputDisplayStickColor);
        static int buttonPressedColor = u8(set->getSettings()->mInputDisplayButtonPressedColor);
        static int buttonColor = u8(set->getSettings()->mInputDisplayButtonColor);
        static int posX = set->getSettings()->mInputDisplayPos.x;
        static int posY = set->getSettings()->mInputDisplayPos.y;

        ImGui::Indent();
        ImGui::Checkbox("Toggle", &set->getSettings()->mIsEnableInputDisplay);

        ImGui::PushItemWidth(200);

        if (ImGui::Combo("Button Color", &buttonColor, sInputDisplayColorNames, IM_ARRAYSIZE(sInputDisplayColorNames)))
            set->getSettings()->mInputDisplayButtonColor = (SettingsMgr::InputDisplayColor)buttonColor;

        if (ImGui::Combo("Pressed Color", &buttonPressedColor, sInputDisplayColorNames, IM_ARRAYSIZE(sInputDisplayColorNames)))
            set->getSettings()->mInputDisplayButtonPressedColor = (SettingsMgr::InputDisplayColor)buttonPressedColor;

        if (ImGui::Combo("Stick Color", &stickColor, sInputDisplayColorNames, IM_ARRAYSIZE(sInputDisplayColorNames)))
            set->getSettings()->mInputDisplayStickColor = (SettingsMgr::InputDisplayColor)stickColor;

        if (ImGui::Combo("Ring Color", &ringColor, sInputDisplayColorNames, IM_ARRAYSIZE(sInputDisplayColorNames)))
            set->getSettings()->mInputDisplayRingColor = (SettingsMgr::InputDisplayColor)ringColor;

        if (ImGui::Combo("BG Color", &bgColor, sInputDisplayColorNames, IM_ARRAYSIZE(sInputDisplayColorNames)))
            set->getSettings()->mInputDisplayBackColor = (SettingsMgr::InputDisplayColor)bgColor;

        if (ImGui::InputInt("Position X", &posX, 5)) set->getSettings()->mInputDisplayPos.x = posX;
        if (ImGui::InputInt("Position Y", &posY, 5)) set->getSettings()->mInputDisplayPos.y = posY;
        ImGui::PopItemWidth();
        if (ImGui::Button("Reset Position")) {
            set->getSettings()->mInputDisplayPos = ImVec2(1600.f / 2, 900.f / 2);
            posX = set->getSettings()->mInputDisplayPos.x;
            posY = set->getSettings()->mInputDisplayPos.y;
        }
        ImGui::Unindent();
    }
    drawHotkeysCat();
    drawInfoCat();

    ImGui::End();
}

void Menu::handleAlways() {
    set = SettingsMgr::instance();
    gameSeq = helpers::tryGetHakoniwaSequence();
    stageScene = helpers::tryGetStageScene(gameSeq);
    // BAAAADD
    if (stageScene) {
        if (strcmp(GameDataFunction::getCurrentStageName(GameDataHolderAccessor(stageScene)), "ClashWorldHomeStage") == 0) noGetPlayer = false;
    }
    if (!noGetPlayer) {
        player = helpers::tryGetPlayerActor(gameSeq);
        playerHak = helpers::tryGetPlayerActorHakoniwa(gameSeq);
    }

    static bool wasMenuDisabled = false;

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
            if (prevNavId) ImGui::SetFocusID(prevNavId, ImGui::FindWindowByName("BTT Studio"));
            GImGui->NavDisableHighlight = false;
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

    bool isAllowTP = set->getSettings()->mIsEnableTpHotkeys && (!InputHelper::isInputToggled() || !mIsEnabledMenu);

    if (InputHelper::isPressPadLeft() && isAllowTP) {
        saveTeleport(tpStates[tpIndex]);
    } else if (InputHelper::isPressPadRight() && isAllowTP) {
        loadTeleport(tpStates[tpIndex]);
    }

    if (isHotkey(set->getSettings()->mKillSceneKey)) {
        if (stageScene) stageScene->kill();
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

    drawInputDisplay();
    drawInfoWindow();
}

void Menu::drawPopup() {
    ImGui::Begin(
        "Popup", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
    );
    ImGui::SetWindowSize(ImVec2(0, 0));
    ImGui::SetWindowPos(ImVec2(mWindowSize.x + 10, 0));
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("%s", popupText);
    ImGui::End();
}

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

void Menu::drawMiscCat() {
    if (ImGui::CollapsingHeader("Wiggler")) {
        ImGui::Indent();
        ImGui::PushItemWidth(200);
        ImGui::Combo("Wiggler Pattern", &set->getSettings()->mWigglerPattern, WigglerPatterns, IM_ARRAYSIZE(WigglerPatterns));
        ImGui::PopItemWidth();
        ImGui::Unindent();
    }
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
                GameDataFunction::addCoin(GameDataHolderWriter(stageScene), -(1000 - coinNum));
            else
                GameDataFunction::addCoin(GameDataHolderWriter(stageScene), -1000);
        }
    }
    if (ImGui::Button("Remove Cappy")) {
        if (playerHak) GameDataFunction::disableCapByPlacement((al::LiveActor*)playerHak->mHackCap);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Noclip", &mIsEnableNoclip);
    ImGui::PushItemWidth(200);
    ImGui::Combo("Moon Refresh Text", &set->getSettings()->mMoonRefreshText, MoonRefreshTexts, IM_ARRAYSIZE(MoonRefreshTexts));
    ImGui::Combo("Menu Corner", &set->getSettings()->mMenuCorner, Corners, IM_ARRAYSIZE(Corners));
    ImGui::PopItemWidth();
}

void Menu::drawHotkeysCat() {
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

void Menu::drawInfoCat() {
    if (ImGui::CollapsingHeader("Info")) {
        ImGui::Indent();
        static int posX = set->getSettings()->mInfoPos.x;
        static int posY = set->getSettings()->mInfoPos.y;
        ImGui::Checkbox("Enable Info Window", &isEnableInfoWindow);
        ImGui::Checkbox("Enable Player Info", &isEnablePlayerInfo);
        if (ImGui::InputInt("Position X##Info", &posX, 5)) set->getSettings()->mInfoPos.x = posX;
        if (ImGui::InputInt("Position Y##Info", &posY, 5)) set->getSettings()->mInfoPos.y = posY;
        if (ImGui::Button("Reset Position##Info")) {
            set->getSettings()->mInfoPos = ImVec2(0, 0);
            posX = set->getSettings()->mInfoPos.x;
            posY = set->getSettings()->mInfoPos.y;
        }

        ImGui::Unindent();
    }
}

inline sead::Vector3f QuatToEuler(sead::Quatf* quat) {
    // Check for null pointer
    if (!quat) {
        // Handle the error, e.g., return a default value or log an error
        return sead::Vector3f(0.0f, 0.0f, 0.0f);
    }

    // Extract quaternion components
    f32 x = quat->z;
    f32 y = quat->y;
    f32 z = quat->x;
    f32 w = quat->w;

    f32 t0 = 2.0f * (w * x + y * z);
    f32 t1 = 1.0f - 2.0f * (x * x + y * y);
    f32 roll = atan2f(t0, t1);
    f32 adjustedRoll = roll;
    if (adjustedRoll < 0) adjustedRoll += M_PI * 2;

    f32 t2 = 2.0f * (w * y - z * x);
    t2 = t2 > 1.0f ? 1.0f : t2;
    t2 = t2 < -1.0f ? -1.0f : t2;
    f32 pitch = asinf(t2);
    f32 adjustedPitch = pitch;
    if (adjustedPitch < 0) adjustedPitch += M_PI * 2;

    f32 t3 = 2.0f * (w * z + x * y);
    f32 t4 = 1.0f - 2.0f * (y * y + z * z);
    f32 yaw = atan2f(t3, t4);
    f32 adjustedYaw = yaw;
    if (adjustedYaw < 0) adjustedYaw += M_PI * 2;

    return sead::Vector3f(adjustedYaw, adjustedPitch, adjustedRoll);
}

void Menu::drawInfoWindow() {
    if (!isEnableInfoWindow) return;
    ImGui::Begin(
        "Info", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
    );
    ImGui::SetWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (!set)
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    else
        ImGui::SetWindowPos(set->getSettings()->mInfoPos, ImGuiCond_Always);

    GameDataHolderAccessor* accessor = helpers::tryGetGameDataHolderAccess();

    if (!accessor) {
        ImGui::Text("No Game Data Holder");
        return;
    }
    s32 jumpCount = rs::getPlayerJumpCount(accessor->mData);
    s32 throwCapCount = rs::getPlayerThrowCapCount(accessor->mData);
    u64 playTimeTotal = GameDataFunction::getPlayTimeTotal(*accessor);
    u64 playTimeAcrossFile = GameDataFunction::getPlayTimeAcrossFile(*accessor);
    s32 totalCoinNum = rs::getTotalCoinNum(accessor->mData);
    ImGui::Text("Jumps: %d", jumpCount);
    ImGui::Text("Cap Throws: %d", throwCapCount);
    ImGui::Text("Total Coins: %d", totalCoinNum);
    ImGui::Text("Play Time Total: %lu", playTimeTotal);
    ImGui::Text("Play Time Across File: %lu", playTimeAcrossFile);

    if (!isEnablePlayerInfo) {
        ImGui::End();
        return;
    }

    if (!playerHak) {
        ImGui::Text("No Player");
        ImGui::End();
        return;
    }
    al::ActorPoseKeeperBase* pose = playerHak->mPoseKeeper;
    if (!pose) {
        ImGui::Text("No Pose");
        ImGui::End();
        return;
    }

    char textBuffer[64];

    float hSpeed = al::calcSpeedH(playerHak), vSpeed = al::calcSpeedV(playerHak), speed = al::calcSpeed(playerHak);
    float hSpeedAngle = atan2f(pose->getVelocityPtr()->z, pose->getVelocityPtr()->x);
    if (hSpeedAngle < 0) hSpeedAngle += M_PI * 2;
    float hSpeedAngleDeg = hSpeedAngle * 180 / M_PI;

    static sead::Vector3f prevPlayerVel = { 0.0f, 0.0f, 0.0f };
    sead::Vector3f playerVelDelta = pose->getVelocity() - prevPlayerVel;

    prevPlayerVel = pose->getVelocity();
    sead::Vector3f playerRot = QuatToEuler(pose->getQuatPtr());

    ImGui::DragFloat3("Trans", &pose->mTrans.x, 50.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
    ImGui::DragFloat3("Velocity", &pose->getVelocityPtr()->x, 1.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
    ImGui::DragFloat3("Vel Delta", &playerVelDelta.x, 1.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);

    snprintf(textBuffer, sizeof(textBuffer), "Speed H: %s", "%.3f");
    ImGui::Text(textBuffer, hSpeed);
    ImGui::SameLine();
    snprintf(textBuffer, sizeof(textBuffer), "V: %s", "%.3f");
    ImGui::Text(textBuffer, vSpeed);
    ImGui::SameLine();
    snprintf(textBuffer, sizeof(textBuffer), "S: %s", "%.3f");
    ImGui::Text(textBuffer, speed);

    snprintf(textBuffer, sizeof(textBuffer), "H Speed Angle: %s", "%.3f");
    ImGui::Text(textBuffer, hSpeedAngleDeg);

    ImGui::DragFloat4("Player Quaternion", &pose->getQuatPtr()->x, 1.f, -1.f, 1.f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
    ImGui::DragFloat3("Euler", &playerRot.x, 1.f, -1.f, 1.f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);

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
