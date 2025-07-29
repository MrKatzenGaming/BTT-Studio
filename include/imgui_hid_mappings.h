#pragma once

#include "imgui.h"
#include "nn/hid.h"

constexpr int npad_mapping[][2] = {
    { ImGuiKey_GamepadFaceDown, static_cast<const int>(nn::hid::NpadButton::A) },
    { ImGuiKey_GamepadFaceRight, static_cast<const int>(nn::hid::NpadButton::B) },
    { ImGuiKey_GamepadFaceUp, static_cast<const int>(nn::hid::NpadButton::X) },
    { ImGuiKey_GamepadFaceLeft, static_cast<const int>(nn::hid::NpadButton::Y) },
    { ImGuiKey_GamepadL1, static_cast<const int>(nn::hid::NpadButton::L) },
    { ImGuiKey_GamepadR1, static_cast<const int>(nn::hid::NpadButton::R) },
    { ImGuiKey_GamepadL2, static_cast<const int>(nn::hid::NpadButton::ZL) },
    { ImGuiKey_GamepadR2, static_cast<const int>(nn::hid::NpadButton::ZR) },
    { ImGuiKey_GamepadStart, static_cast<const int>(nn::hid::NpadButton::Plus) },
    { ImGuiKey_GamepadBack, static_cast<const int>(nn::hid::NpadButton::Minus) },
    { ImGuiKey_GamepadDpadLeft, static_cast<const int>(nn::hid::NpadButton::Left) },
    { ImGuiKey_GamepadDpadRight, static_cast<const int>(nn::hid::NpadButton::Right) },
    { ImGuiKey_GamepadDpadUp, static_cast<const int>(nn::hid::NpadButton::Up) },
    { ImGuiKey_GamepadDpadDown, static_cast<const int>(nn::hid::NpadButton::Down) },
    { ImGuiKey_GamepadLStickLeft, static_cast<const int>(nn::hid::NpadButton::StickLLeft) },
    { ImGuiKey_GamepadLStickRight, static_cast<const int>(nn::hid::NpadButton::StickLRight) },
    { ImGuiKey_GamepadLStickUp, static_cast<const int>(nn::hid::NpadButton::StickLUp) },
    { ImGuiKey_GamepadLStickDown, static_cast<const int>(nn::hid::NpadButton::StickLDown) },
};

constexpr int mouse_mapping[][2] = {
    { ImGuiMouseButton_Left, static_cast<const int>(nn::hid::MouseButton::Left) },
    { ImGuiMouseButton_Right, static_cast<const int>(nn::hid::MouseButton::Right) },
    { ImGuiMouseButton_Middle, static_cast<const int>(nn::hid::MouseButton::Middle) },
};
