#include "imgui.h"
#include "imgui_internal.h"
#include "Menu.h"

using namespace btt;

void Menu::drawPageOptions() {
    if (ImGui::CollapsingHeader("Options")) {
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Refresh")) {
            ImGui::Indent();
            ImGui::Checkbox("Moon Refresh", &set->getSettings()->mIsEnableMoonRefresh);
            ImGui::Checkbox("Always Manually Skip Cutscene", &set->getSettings()->mIsEnableAlwaysManualCutscene);
            ImGui::Checkbox("Refresh Warp Text", &set->getSettings()->mIsEnableRefreshWarpText);
            ImGui::Checkbox("Refresh Purple Coins", &set->getSettings()->mIsEnableRefreshPurps);
            ImGui::Checkbox("Refresh Doors", &set->getSettings()->mIsEnableDoorRefresh);
            ImGui::Checkbox("Refresh Seeds", &set->getSettings()->mIsEnableFlowerPotRefresh);
            ImGui::Checkbox("Refresh Toad Text", &set->getSettings()->mIsEnableRefreshNpc);
            ImGui::Checkbox("Disable Saving Coin Stacks", &set->getSettings()->mIsEnableDisableCoinStackSave);
            ImGui::Checkbox("Refresh Kingdom Enter Cutscenes", &set->getSettings()->mIsEnableRefreshKingdomEnter);
            if (GImGui->NavId == ImGui::GetID("Refresh Kingdom Enter Cutscenes")) {
                ImGui::SetTooltip("Doesn't refresh Cappy text after landing in Cascade, Sand, Metro, Lost and Moon.");
            }

#ifndef BTTDEBUG
            ImGui::BeginDisabled();
#endif
            ImGui::Checkbox("Refresh Moon Shards", &set->getSettings()->mIsEnableShardRefresh);
#ifndef BTTDEBUG
            ImGui::EndDisabled();
#endif
            ImGui::Unindent();
        }
        if (ImGui::CollapsingHeader("Disable")) {
            ImGui::Indent();
            ImGui::Checkbox("Disable Auto Save", &set->getSettings()->mIsEnableDisableAutoSave);
            ImGui::Checkbox("Disable Moon Lock", &set->getSettings()->mIsEnableDisableMoonLock);
            ImGui::Checkbox("No Damage", &set->getSettings()->mIsEnableNoDamage);
            ImGui::Checkbox("Disable Music", &set->getSettings()->mIsEnableDisableMusic);
            ImGui::Checkbox("Disable Teleport Puppet", &set->getSettings()->mIsEnableDisableTpPuppet);
            if (GImGui->NavId == ImGui::GetID("Disable Teleport Puppet")) ImGui::SetTooltip("Only in Moon Get Animation");
            ImGui::Checkbox("No Checkpoint Touch", &set->getSettings()->mIsEnableNoCheckpointTouch);
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Misc##options")) {
            ImGui::Indent();
            ImGui::Checkbox("Always Allow Checkpoints", &set->getSettings()->mIsEnableAlwaysCheckpoints);
            ImGui::Checkbox("Skip Cloud", &set->getSettings()->mIsEnableSkipCloud);
            ImGui::Checkbox("Enable All Checkpoints", &set->getSettings()->mIsEnableAllCheckpoints);
            ImGui::Checkbox("Spawn all Hint Art Moons", &set->getSettings()->mIsEnableHintPhotoSpawn);

            ImGui::Unindent();
        }

        ImGui::Unindent();
    }
}
