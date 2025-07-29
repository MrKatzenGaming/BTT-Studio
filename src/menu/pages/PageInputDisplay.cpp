#include "imgui.h"
#include "menu/InputDisplay.h"
#include "menu/Menu.h"

using namespace btt;

void Menu::drawPageInputDisplay() {
    if (ImGui::CollapsingHeader("Input Display")) {
        static int bgColor = u8(set->getSettings()->mInputDisplayBackColor);
        static int ringColor = u8(set->getSettings()->mInputDisplayRingColor);
        static int stickColor = u8(set->getSettings()->mInputDisplayStickColor);
        static int buttonPressedColor = u8(set->getSettings()->mInputDisplayButtonPressedColor);
        static int buttonColor = u8(set->getSettings()->mInputDisplayButtonColor);
        static int posX = set->getSettings()->mInputDisplayPos.x;
        static int posY = set->getSettings()->mInputDisplayPos.y;
        static int posXP2 = set->getSettings()->mInputDisplayPosP2.x;
        static int posYP2 = set->getSettings()->mInputDisplayPosP2.y;

        ImGui::Indent();
        ImGui::Checkbox("Toggle", &set->getSettings()->mIsEnableInputDisplay);
        ImGui::Checkbox("2P Mode", &set->getSettings()->mIsEnableInput2P);

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

        ImGui::SeparatorText("Player 2");
        ImGui::PushItemWidth(200);

        if (ImGui::InputInt("Position X P2", &posXP2, 5)) set->getSettings()->mInputDisplayPosP2.x = posXP2;
        if (ImGui::InputInt("Position Y P2", &posYP2, 5)) set->getSettings()->mInputDisplayPosP2.y = posYP2;
        ImGui::PopItemWidth();
        if (ImGui::Button("Reset Position P2")) {
            set->getSettings()->mInputDisplayPosP2 = ImVec2(1600.f / 2, 900.f / 2);
            posXP2 = set->getSettings()->mInputDisplayPosP2.x;
            posYP2 = set->getSettings()->mInputDisplayPosP2.y;
        }

        ImGui::Unindent();
    }
}
