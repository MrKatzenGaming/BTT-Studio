#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"

#include "game/System/GameDataFunction.h"
#include "game/Util/AchievementUtil.h"

#include "getHelper.h"
#include "Menu.h"

using namespace btt;

void Menu::drawPageInfo() {
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

sead::Vector3f QuatToEuler(sead::Quatf* quat) {
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
