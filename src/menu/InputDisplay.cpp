#include "InputDisplay.h"

#include "al/Library/Controller/InputFunction.h"
#include "al/Library/Pad/NpadController.h"

#include "sead/controller/seadControllerMgr.h"
#include <sead/math/seadVector.h>

#include "game/Player/PlayerInputFunction.h"

#include "getHelper.h"
#include "imgui_internal.h"

namespace btt {

using InputCallback = bool;

static ImU32 makeColor(const ImVec4& color) {
    return IM_COL32(u8(color.x), u8(color.y), u8(color.z), u8(color.w));
}

static void drawButton(
    const ImVec2& pos, InputCallback callback, float radius = 8,
    ImU32 color = makeColor(getInputDisplayColor(SettingsMgr::instance()->getSettings()->mInputDisplayButtonColor)),
    ImU32 pressedColor = makeColor(getInputDisplayColor(SettingsMgr::instance()->getSettings()->mInputDisplayButtonPressedColor))
) {
    const ImU32 col = callback ? pressedColor : color;
    ImGui::GetForegroundDrawList()->AddCircleFilled(pos, radius, col, 16);
}

static void drawButtonRect(
    const ImVec2& pos, InputCallback callback, ImU32 color = makeColor(getInputDisplayColor(SettingsMgr::instance()->getSettings()->mInputDisplayButtonColor)),
    ImU32 pressedColor = makeColor(getInputDisplayColor(SettingsMgr::instance()->getSettings()->mInputDisplayButtonPressedColor))
) {
    const ImU32 col = callback ? pressedColor : color;
    const ImVec2 min(pos.x - 13, pos.y - 6);
    const ImVec2 max(pos.x + 13, pos.y + 6);
    ImGui::GetForegroundDrawList()->AddRectFilled(min, max, col, 10.0f);
}

void drawInputDisplay() {
    SettingsMgr* set = SettingsMgr::instance();
    if (!set->getSettings()->mIsEnableInputDisplay) return;

    sead::ControllerMgr* controllerMgr = sead::ControllerMgr::instance();
    al::NpadController* controller = (al::NpadController*)controllerMgr->getController(al::getPlayerControllerPort(0));

    const sead::Vector2f leftStick = { controller->mLeftStick.x, controller->mLeftStick.y };
    const sead::Vector2f rightStick = { controller->mRightStick.x, controller->mRightStick.y };

    ImVec2 pos = set->getSettings()->mInputDisplayPos;
    pos.x -= 200;
    pos.y -= 100;

    if (set->getSettings()->mIsEnableInput2P) {
        ImGui::GetForegroundDrawList()->AddText(
            ImGui::GetDefaultFont(), 27.f, { pos.x + 20, pos.y - 100 }, makeColor(getInputDisplayColor(SettingsMgr::InputDisplayColor::White)), "Player 1"
        );
    }

    if (set->getSettings()->mInputDisplayBackColor != SettingsMgr::InputDisplayColor::None) {
        ImVec4 color = getInputDisplayColor(set->getSettings()->mInputDisplayBackColor);
        color.w = 128;
        ImGui::GetForegroundDrawList()->AddRectFilled({ pos.x - 50, pos.y - 100 }, { pos.x + 200, pos.y + 100 }, makeColor(color), 20.0f);
    }

    ImGui::GetForegroundDrawList()->AddCircle(pos, 25, makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayRingColor)), 0, 2);
    ImVec2 leftPos = { pos.x + leftStick.x * 30, pos.y - leftStick.y * 30 };
    drawButton(
        leftPos, controller->mPadHold.isOnBit(controller->cPadIdx_1) /*StickL*/, 16,
        makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayStickColor))
    );

    pos.x += 40;
    pos.y += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Up)); // up
    pos.y += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Down)); // down
    pos.x -= 15;
    pos.y -= 15;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Left)); // left
    pos.x += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Right)); // right

    pos.x += 60;
    ImGui::GetForegroundDrawList()->AddCircle(pos, 25, makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayRingColor)), 0, 2);
    ImVec2 rightPos = { pos.x + rightStick.x * 30, pos.y - rightStick.y * 30 };
    drawButton(
        rightPos, controller->mPadHold.isOnBit(controller->cPadIdx_2) /*StickR*/, 16,
        makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayStickColor))
    );

    pos.x += 40;
    pos.y -= 60;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_X)); // x
    pos.y += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_B)); // b
    pos.x -= 15;
    pos.y -= 15;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Y)); // y
    pos.x += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_A)); // a

    pos.y -= 10;
    pos.x -= 75;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Plus), 5); // plus
    pos.x -= 40;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Minus), 5); // minus

    pos = set->getSettings()->mInputDisplayPos;
    pos.x -= 200;
    pos.y -= 160;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_L)); // l
    pos.y -= 16;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_ZL)); // zl
    pos.x += 155;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_ZR)); // zr
    pos.y += 16;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_R)); // r
}

void drawInputDisplayP2() {
    SettingsMgr* set = SettingsMgr::instance();
    if (!set->getSettings()->mIsEnableInputDisplay || !set->getSettings()->mIsEnableInput2P) return;

    sead::ControllerMgr* controllerMgr = sead::ControllerMgr::instance();
    al::NpadController* controller = (al::NpadController*)controllerMgr->getController(al::getPlayerControllerPort(1));

    const sead::Vector2f leftStick = { controller->mLeftStick.x, controller->mLeftStick.y };
    const sead::Vector2f rightStick = { controller->mRightStick.x, controller->mRightStick.y };

    ImVec2 pos = set->getSettings()->mInputDisplayPosP2;
    pos.x -= 200;
    pos.y -= 100;

    ImGui::GetForegroundDrawList()->AddText(
        ImGui::GetDefaultFont(), 27.f, { pos.x + 20, pos.y - 100 }, makeColor(getInputDisplayColor(SettingsMgr::InputDisplayColor::White)), "Player 2"
    );

    if (set->getSettings()->mInputDisplayBackColor != SettingsMgr::InputDisplayColor::None) {
        ImVec4 color = getInputDisplayColor(set->getSettings()->mInputDisplayBackColor);
        color.w = 128;
        ImGui::GetForegroundDrawList()->AddRectFilled({ pos.x - 50, pos.y - 100 }, { pos.x + 200, pos.y + 100 }, makeColor(color), 20.0f);
    }

    ImGui::GetForegroundDrawList()->AddCircle(pos, 25, makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayRingColor)), 0, 2);
    ImVec2 leftPos = { pos.x + leftStick.x * 30, pos.y - leftStick.y * 30 };
    drawButton(
        leftPos, controller->mPadHold.isOnBit(controller->cPadIdx_1) /*StickL*/, 16,
        makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayStickColor))
    );

    pos.x += 40;
    pos.y += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Up)); // up
    pos.y += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Down)); // down
    pos.x -= 15;
    pos.y -= 15;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Left)); // left
    pos.x += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Right)); // right

    pos.x += 60;
    ImGui::GetForegroundDrawList()->AddCircle(pos, 25, makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayRingColor)), 0, 2);
    ImVec2 rightPos = { pos.x + rightStick.x * 30, pos.y - rightStick.y * 30 };
    drawButton(
        rightPos, controller->mPadHold.isOnBit(controller->cPadIdx_2) /*StickR*/, 16,
        makeColor(getInputDisplayColor(set->getSettings()->mInputDisplayStickColor))
    );

    pos.x += 40;
    pos.y -= 60;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_X)); // x
    pos.y += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_B)); // b
    pos.x -= 15;
    pos.y -= 15;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Y)); // y
    pos.x += 30;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_A)); // a

    pos.y -= 10;
    pos.x -= 75;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Plus), 5); // plus
    pos.x -= 40;
    drawButton(pos, controller->mPadHold.isOnBit(controller->cPadIdx_Minus), 5); // minus

    pos = set->getSettings()->mInputDisplayPosP2;
    pos.x -= 200;
    pos.y -= 160;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_L)); // l
    pos.y -= 16;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_ZL)); // zl
    pos.x += 155;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_ZR)); // zr
    pos.y += 16;
    drawButtonRect(pos, controller->mPadHold.isOnBit(controller->cPadIdx_R)); // r
}

} // namespace btt
