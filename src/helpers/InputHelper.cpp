#include "InputHelper.h"

#include "hid_custom.h"
#include "menu/Menu.h"

static const char* styleNames[] = {
    "Pro Controller",
    "Joy-Con controller in handheld mode",
    "Joy-Con controller in dual mode",
    "Joy-Con left controller in single mode",
    "Joy-Con right controller in single mode",
    "GameCube controller",
    "Poké Ball Plus controller",
    "NES/Famicom controller",
    "NES/Famicom controller in handheld mode",
    "SNES controller",
    "N64 controller",
    "Sega Genesis controller",
    "generic external controller",
    "generic controller",
};

nn::hid::NpadBaseState InputHelper::prevControllerState {};
nn::hid::NpadBaseState InputHelper::curControllerState {};

nn::hid::KeyboardState InputHelper::curKeyboardState {};
nn::hid::KeyboardState InputHelper::prevKeyboardState {};

nn::hid::MouseState InputHelper::curMouseState {};
nn::hid::MouseState InputHelper::prevMouseState {};

nn::hid::TouchScreenState<1> InputHelper::curTouchState {};
nn::hid::TouchScreenState<1> InputHelper::prevTouchState {};

ulong InputHelper::selectedPort = -1;
bool InputHelper::isReadInput = true;
bool InputHelper::toggleInput = false;
bool InputHelper::disableMouse = false;

const char* getStyleName(nn::hid::NpadStyleSet style) {
    s32 index = -1;

    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleFullKey)) {
        index = 0;
    }
    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleHandheld)) {
        index = 1;
    }
    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleJoyDual)) {
        index = 2;
    }
    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleJoyLeft)) {
        index = 3;
    }
    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleJoyRight)) {
        index = 4;
    }
    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleSystemExt)) {
        index = 12;
    }
    if (style.Test((int)nn::hid::NpadStyleTag::NpadStyleSystem)) {
        index = 13;
    }

    if (index != -1) {
        return styleNames[index];
    } else {
        return "Unknown";
    }
}

void InputHelper::initKBM() {
    nn::hid::InitializeKeyboard();
    nn::hid::InitializeMouse();
}

void InputHelper::updatePadState() {
    prevControllerState = curControllerState;
    tryGetContState(&curControllerState, selectedPort);

    prevKeyboardState = curKeyboardState;
    nn::hid::GetKeyboardState(&curKeyboardState);

    prevMouseState = curMouseState;
    nn::hid::GetMouseState(&curMouseState);

    prevTouchState = curTouchState;
    nn::hid::GetTouchScreenState(&curTouchState);

    if (isHoldR() && isHoldZR() && isPressL() && btt::Menu::instance()->mIsEnabledMenu) {
        toggleInput = !toggleInput;
    }
}

bool InputHelper::tryGetContState(nn::hid::NpadBaseState* state, ulong port) {
    nn::hid::NpadStyleSet styleSet = nn::hid::GetNpadStyleSet(port);
    isReadInput = true;
    bool result = true;

    if (styleSet.Test((int)nn::hid::NpadStyleTag::NpadStyleFullKey)) {
        nn::hid::GetNpadState((nn::hid::NpadFullKeyState*)state, port);
    } else if (styleSet.Test((int)nn::hid::NpadStyleTag::NpadStyleHandheld)) {
        nn::hid::GetNpadState((nn::hid::NpadHandheldState*)state, port);
    } else if (styleSet.Test((int)nn::hid::NpadStyleTag::NpadStyleJoyDual)) {
        nn::hid::GetNpadState((nn::hid::NpadJoyDualState*)state, port);
    } else if (styleSet.Test((int)nn::hid::NpadStyleTag::NpadStyleJoyLeft)) {
        nn::hid::GetNpadState((nn::hid::NpadJoyLeftState*)state, port);
    } else if (styleSet.Test((int)nn::hid::NpadStyleTag::NpadStyleJoyRight)) {
        nn::hid::GetNpadState((nn::hid::NpadJoyRightState*)state, port);
    } else {
        result = false;
    }

    isReadInput = false;

    return result;
}

bool InputHelper::isButtonHold(nn::hid::NpadButton button) {
    return curControllerState.mButtons.Test((int)button);
}

bool InputHelper::isButtonPress(nn::hid::NpadButton button) {
    return curControllerState.mButtons.Test((int)button) && !prevControllerState.mButtons.Test((int)button);
}

bool InputHelper::isButtonRelease(nn::hid::NpadButton button) {
    return !curControllerState.mButtons.Test((int)button) && prevControllerState.mButtons.Test((int)button);
}

bool InputHelper::isKeyHold(nn::hid::KeyboardKey key) {
    return curKeyboardState.mKeys.Test((int)key);
}

bool InputHelper::isKeyPress(nn::hid::KeyboardKey key) {
    return curKeyboardState.mKeys.Test((int)key) && !prevKeyboardState.mKeys.Test((int)key);
}

bool InputHelper::isKeyRelease(nn::hid::KeyboardKey key) {
    return !curKeyboardState.mKeys.Test((int)key) && prevKeyboardState.mKeys.Test((int)key);
}

bool InputHelper::isModifierActive(nn::hid::KeyboardModifier modifier) {
    return curKeyboardState.mModifiers.Test((int)modifier);
}

bool InputHelper::isMouseHold(nn::hid::MouseButton button) {
    return curMouseState.mButtons.Test((int)button);
}

bool InputHelper::isMousePress(nn::hid::MouseButton button) {
    return curMouseState.mButtons.Test((int)button) && !prevMouseState.mButtons.Test((int)button);
}

bool InputHelper::isMouseRelease(nn::hid::MouseButton button) {
    return !curMouseState.mButtons.Test((int)button) && prevMouseState.mButtons.Test((int)button);
}

bool InputHelper::isMouseConnected() {
    if (!disableMouse)
        return curMouseState.mAttributes.Test((int)nn::hid::MouseAttribute::IsConnected);
    else
        return false;
}

void InputHelper::getMouseCoords(float* x, float* y) {
    *x = curMouseState.mX;
    *y = curMouseState.mY;
}

void InputHelper::getScrollDelta(float* x, float* y) {
    *x = curMouseState.mWheelDeltaX;
    *y = curMouseState.mWheelDeltaY;
}

bool InputHelper::getTouchCoords(s32* x, s32* y) {
    if (curTouchState.mCount > 0) {
        *x = curTouchState.mTouches[0].mX;
        *y = curTouchState.mTouches[0].mY;
        return true;
    }
    return false;
}

bool InputHelper::isHoldTouch() {
    return curTouchState.mCount > 0;
}

bool InputHelper::isPressTouch() {
    return curTouchState.mCount > 0 && prevTouchState.mCount == 0;
}

bool InputHelper::isReleaseTouch() {
    return curTouchState.mCount == 0 && prevTouchState.mCount > 0;
}
