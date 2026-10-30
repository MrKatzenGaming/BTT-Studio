#include "helpers/ImGuiHelper.h"
#include "imgui.h"
#include "menu/Menu.h"

using namespace btt;

void Menu::drawPageOptions() {
    if (ImGui::CollapsingHeader("Options")) {
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Refresh##options")) {
            ImGui::Indent();
            ImGuiHelper::CheckboxDesc(
                "Disable Saving Moons", "Moon Grabs are no longer Saved", // TODO: sounds weird idk
                &set->getSettings()->mIsEnableMoonRefresh
            );
            ImGuiHelper::CheckboxDesc(
                "Refresh Gray Moons",
                "Grabbed Moons will now be back in Default State", // TODO: sounds weird idk
                &set->getSettings()->mIsEnableGrayMoonRefresh
            );
            ImGuiHelper::CheckboxDesc(
                "Always Manually Skip Cutscene",
                "Always get the Skip Cutscene prompt even if the cutscene has been watched before",
                &set->getSettings()->mIsEnableAlwaysManualCutscene
            );
            ImGuiHelper::CheckboxDesc(
                "Refresh Warp Text",
                "Text explaining warps when you first open the map will now always appear",
                &set->getSettings()->mIsEnableRefreshWarpText
            );
            ImGui::Checkbox("Refresh Purple Coins", &set->getSettings()->mIsEnableRefreshPurps);
            ImGuiHelper::CheckboxDesc(
                "Refresh Doors", "Cap Doors are now always closed", &set->getSettings()->mIsEnableDoorRefresh
            );
            ImGuiHelper::CheckboxDesc(
                "Refresh Seeds", "Seeds will always be unplanted after a scene reload",
                &set->getSettings()->mIsEnableFlowerPotRefresh
            );
            ImGuiHelper::CheckboxDesc(
                "Refresh Toad Text",
                "Before Moon Grab Toad Text now Refresh", // TODO: prolly no desc, idek how this works anymore
                &set->getSettings()->mIsEnableRefreshNpc
            );
            ImGuiHelper::CheckboxDesc(
                "Disable Saving Coin Stacks", "Coin Stacks will full reappear after a scene reload",
                &set->getSettings()->mIsEnableDisableCoinStackSave
            );
            ImGui::Checkbox(
                "Refresh Kingdom Enter Cutscenes", &set->getSettings()->mIsEnableRefreshKingdomEnter
            );
            ImGui::Checkbox("Refresh Moon Shards", &set->getSettings()->mIsEnableShardRefresh);

            ImGui::Unindent();
        }
        if (ImGui::CollapsingHeader("Disable##options")) {
            ImGui::Indent();
            ImGui::Checkbox("Disable Auto Save", &set->getSettings()->mIsEnableDisableAutoSave);
            ImGuiHelper::CheckboxDesc(
                "Disable Moon Lock", "Moon Locked will be Disabled",
                &set->getSettings()->mIsEnableDisableMoonLock
            );
            ImGui::Checkbox("No Damage", &set->getSettings()->mIsEnableNoDamage);
            ImGui::Checkbox("Disable Music", &set->getSettings()->mIsEnableDisableMusic);
            ImGuiHelper::CheckboxDesc(
                "Disable Teleport Puppet", "Only in moon get animation",
                &set->getSettings()->mIsEnableDisableTpPuppet
            );
            ImGuiHelper::CheckboxDesc(
                "No Checkpoint Touch", "Touched Checkpoints don't set your respawn point",
                &set->getSettings()->mIsEnableNoCheckpointTouch
            );
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Misc##options")) {
            ImGui::Indent();
            ImGui::Checkbox("Always Allow Warping", &set->getSettings()->mIsEnableAlwaysCheckpoints);
            ImGui::Checkbox("Skip Cloud", &set->getSettings()->mIsEnableSkipCloud);
            ImGui::Checkbox("Enable All Checkpoints", &set->getSettings()->mIsEnableAllCheckpoints);
            ImGui::Checkbox("Spawn all Hint Art Moons", &set->getSettings()->mIsEnableHintPhotoSpawn);

            ImGui::Unindent();
        }

        ImGui::Unindent();
    }
}
