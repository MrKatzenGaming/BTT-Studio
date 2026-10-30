#include "imgui.h"
#include "imgui_internal.h"

namespace ImGuiHelper {

bool CheckboxDesc(const char* label, const char* desc, bool* v) {
    bool b = ImGui::Checkbox(label, v);
    if (GImGui->NavId == ImGui::GetID(label)) {
        ImGui::SetTooltip("%s", desc);
    }
    return b;
}

} // namespace ImGuiHelper
