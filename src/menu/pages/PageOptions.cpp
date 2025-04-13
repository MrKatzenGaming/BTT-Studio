#include "Menu.h"

using namespace btt;

void Menu::drawPageOptions() {
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
}
