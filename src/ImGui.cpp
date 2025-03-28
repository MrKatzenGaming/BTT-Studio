#include "ImGui.h"
#include "hk/diag/diag.h"
#include "hk/gfx/ImGuiBackendNvn.h"

#include "nn/hid.h"
#include "nn/oe.h"

#include "helpers/InputHelper.h"
#include "helpers/fsHelper.h"
#include "menu/Menu.h"

#include "imgui_hid_mappings.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace btt::imgui {

    ImVector<ProcDrawFunc> drawQueue;
    
    void init(sead::Heap* heap) {
        auto* imgui = hk::gfx::ImGuiBackendNvn::instance();

        static sead::Heap* sImHeap = heap;

        imgui->setAllocator(
            { [](size allocSize, size alignment) -> void* {
                 return sImHeap->tryAlloc(allocSize, alignment);
             },
                [](void* ptr) -> void {
                    sImHeap->free(ptr);
                } });
        imgui->tryInitialize();

        InputHelper::initKBM();
        InputHelper::setPort(0); // set input helpers default port to zero
    }

    void updateTouch(ImGuiIO& io) {
        static s32 touchPosX = 0;
        static s32 touchPosY = 0;
        InputHelper::getTouchCoords(&touchPosX, &touchPosY);
        io.AddMousePosEvent(touchPosX, touchPosY);
    
        ImGuiMouseButton button = ImGuiMouseButton_Left;
    
        if (InputHelper::isPressTouch())
            io.AddMouseButtonEvent(button, true);
        else if (InputHelper::isReleaseTouch())
            io.AddMouseButtonEvent(button, false);
    }
    
    void updateMouse(ImGuiIO& io) {
        ImVec2 mousePos(0, 0);
        InputHelper::getMouseCoords(&mousePos.x, &mousePos.y);
    
        mousePos = ImVec2((mousePos.x / 1280.f) * io.DisplaySize.x, (mousePos.y / 720.f) * io.DisplaySize.y);
    
        io.AddMousePosEvent(mousePos.x, mousePos.y);
    
        ImVec2 scrollDelta(0, 0);
        InputHelper::getScrollDelta(&scrollDelta.x, &scrollDelta.y);
    
        if (scrollDelta.x != 0.0f) io.AddMouseWheelEvent(0.0f, scrollDelta.x > 0.0f ? 0.5f : -0.5f);
    
        for (auto [im_k, nx_k] : mouse_mapping) {
            if (InputHelper::isMousePress((nn::hid::MouseButton)nx_k))
                io.AddMouseButtonEvent((ImGuiMouseButton)im_k, true);
            else if (InputHelper::isMouseRelease((nn::hid::MouseButton)nx_k))
                io.AddMouseButtonEvent((ImGuiMouseButton)im_k, false);
        }
    }

    void updateGamepad(ImGuiIO& io) {
        for (auto [im_k, nx_k] : npad_mapping) {
            if (InputHelper::isButtonPress((nn::hid::NpadButton)nx_k))
                io.AddKeyEvent((ImGuiKey)im_k, true);
            else if (InputHelper::isButtonRelease((nn::hid::NpadButton)nx_k))
                io.AddKeyEvent((ImGuiKey)im_k, false);
        }
    }

    void updateInput() {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = InputHelper::isMouseConnected();
    
        if (io.MouseDrawCursor) updateMouse(io);
    
        if (nn::oe::GetOperationMode() == nn::oe::OperationMode_Handheld) updateTouch(io);
    
        if (InputHelper::isInputToggled() && btt::Menu::instance()->mIsEnabledMenu) updateGamepad(io);
    }

    void addDrawFunc(ProcDrawFunc func) {

        HK_ASSERT(!drawQueue.contains(func));//, "Function has already been added to queue!"

        drawQueue.push_back(func);
    }

    void draw(agl::DrawContext* drawContext) {
        InputHelper::updatePadState();
        updateInput();
    
        ImGui::NewFrame();
        for (auto drawFunc : drawQueue) {
            drawFunc();
        }
        ImGui::Render();
    
        hk::gfx::ImGuiBackendNvn::instance()->draw(ImGui::GetDrawData(), drawContext->getCommandBuffer()->ToData()->pNvnCommandBuffer);
    }

    void setupStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
    
        style.IndentSpacing = 10;
    
        // Change the color of the title bar
        colors[ImGuiCol_TitleBg] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    
        // Change the color of the frame background
        colors[ImGuiCol_FrameBg] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.83f, 0.44f, 0.5f, 1.f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    
        // Change the color of the button
        colors[ImGuiCol_Button] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.83f, 0.44f, 0.5f, 1.f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    
        colors[ImGuiCol_Header] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.83f, 0.44f, 0.5f, 1.f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.73f, 0.34f, 0.4f, 1.f);
    
        colors[ImGuiCol_NavHighlight] = ImVec4(1, 1, 1, 1);

        ImGuiIO& io = ImGui::GetIO();

        io.MouseDrawCursor = InputHelper::isMouseConnected();
        io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    }
}