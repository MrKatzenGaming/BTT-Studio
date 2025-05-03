#include <hk/gfx/ImGuiBackendNvn.h>
#include <hk/hook/Trampoline.h>

#include <al/Library/Memory/HeapUtil.h>
#include <al/Library/System/GameSystemInfo.h>

#include <sead/filedevice/seadFileDeviceMgr.h>
#include <sead/heap/seadExpHeap.h>

#include <game/Sequence/HakoniwaSequence.h>
#include <game/System/Application.h>
#include <game/System/GameSystem.h>

#include <nn/fs.h>

#include "helpers/InputHelper.h"
#include "ImGui.h"
#include "InputDisplay.h"
#include "logger.h"
#include "Menu.h"
#include "saveFileHelper.h"
#include "settings/SettingsHooks.h"
#include "settings/SettingsMgr.h"

using namespace hk;

namespace btt {
sead::Heap* initializeHeap() {
    return sead::ExpHeap::create(3_MB, "BTTStudioHeap", al::getStationedHeap(), 8, sead::Heap::cHeapDirection_Forward, false);
}
} // namespace btt

HkTrampoline<void, GameSystem*> gameSystemInit = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    sead::Heap* heap = btt::initializeHeap();

    Logger* logger = Logger::createInstance(heap);
    logger->init(heap);

#ifdef DEBUG
    logger->connect("192.168.178.41", 8171);
#endif

    btt::SettingsMgr::createInstance(heap);
    logger->log(Logger::LogType::LogInfo, "SettingsMgr instance created");
    btt::Menu* menu = btt::Menu::createInstance(heap);
    logger->log(Logger::LogType::LogInfo, "Menu instance created");
    SaveFileHelper* save = SaveFileHelper::createInstance(heap);
    logger->log(Logger::LogType::LogInfo, "SaveFileHelper instance created");
    save->init(heap);
    logger->log(Logger::LogType::LogInfo, "SaveFileHelper initialized");
    save->loadSettings(heap);
    save->loadTeleport(menu->tpStates, hk::util::arraySize(menu->tpStates), heap);
    save->mSaveThread->start();
    logger->log(Logger::LogType::LogInfo, "SaveFileHelper setup done");

    btt::imgui::init(heap);
    logger->log(Logger::LogType::LogInfo, "ImGui initialized");
    btt::imgui::setupStyle();
    btt::imgui::addDrawFunc([] {
        btt::Menu* menu = btt::Menu::instance();
        if (menu) {
            if (menu->mIsEnabledMenu) menu->draw();
            menu->drawInfoWindow();
            if (btt::SettingsMgr::instance()->getSettings()->mIsEnableInputDisplay) {
                btt::drawInputDisplay();
                if (btt::SettingsMgr::instance()->getSettings()->mIsEnableInput2P) btt::drawInputDisplayP2();
            }
        }
    });
    logger->log(Logger::LogType::LogInfo, "ImGui setup done");

    InputHelper::setDisableMouse(true);

    gameSystemInit.orig(gameSystem);

    logger->log(Logger::LogType::LogInfo, "GameSystem initialized");
});

HkTrampoline<void, GameSystem*> drawMainHook = hk::hook::trampoline([](GameSystem* gameSystem) -> void {
    drawMainHook.orig(gameSystem);

    auto drawContext = Application::instance()->mDrawSystemInfo->drawContext;
    btt::imgui::draw(drawContext);
});

HkTrampoline<void, sead::FileDeviceMgr*> fileDeviceMgrHook = hk::hook::trampoline([](sead::FileDeviceMgr* fileDeviceMgr) -> void {
    fileDeviceMgrHook.orig(fileDeviceMgr);

    fileDeviceMgr->mMountedSd = nn::fs::MountSdCardForDebug("sd") == 0;
});

int timer = 0;
HkTrampoline<void, HakoniwaSequence*> hakoniwaSequenceUpdate = hk::hook::trampoline([](HakoniwaSequence* hakoniwaSequence) -> void {
    hakoniwaSequenceUpdate.orig(hakoniwaSequence);

    btt::Menu::instance()->handleAlways();
});

void disableButtons(nn::hid::NpadBaseState* state) {
    if (!InputHelper::isReadInputs() && InputHelper::isInputToggled() && btt::Menu::instance()->mIsEnabledMenu) {
        // clear out the data within the state (except for the sampling number and attributes)
        state->mButtons = nn::hid::NpadButtonSet();
        state->mAnalogStickL = nn::hid::AnalogStickState();
        state->mAnalogStickR = nn::hid::AnalogStickState();
    }
}

HkTrampoline<int, int*, nn::hid::NpadFullKeyState*, int, const unsigned int&> DisableFullKeyState =
    hk::hook::trampoline([](int* unkInt, nn::hid::NpadFullKeyState* state, int count, const unsigned int& port) -> int {
        int result = DisableFullKeyState.orig(unkInt, state, count, port);
        disableButtons(state);
        return result;
    });

HkTrampoline<int, int*, nn::hid::NpadHandheldState*, int, const unsigned int&> DisableHandheldState =
    hk::hook::trampoline([](int* unkInt, nn::hid::NpadHandheldState* state, int count, const unsigned int& port) -> int {
        int result = DisableHandheldState.orig(unkInt, state, count, port);
        disableButtons(state);
        return result;
    });

HkTrampoline<int, int*, nn::hid::NpadJoyDualState*, int, const unsigned int&> DisableJoyDualState =
    hk::hook::trampoline([](int* unkInt, nn::hid::NpadJoyDualState* state, int count, const unsigned int& port) -> int {
        int result = DisableJoyDualState.orig(unkInt, state, count, port);
        disableButtons(state);
        return result;
    });

HkTrampoline<int, int*, nn::hid::NpadJoyLeftState*, int, const unsigned int&> DisableJoyLeftState =
    hk::hook::trampoline([](int* unkInt, nn::hid::NpadJoyLeftState* state, int count, const unsigned int& port) -> int {
        int result = DisableJoyLeftState.orig(unkInt, state, count, port);
        disableButtons(state);
        return result;
    });

HkTrampoline<int, int*, nn::hid::NpadJoyRightState*, int, const unsigned int&> DisableJoyRightState =
    hk::hook::trampoline([](int* unkInt, nn::hid::NpadJoyRightState* state, int count, const unsigned int& port) -> int {
        int result = DisableJoyRightState.orig(unkInt, state, count, port);
        disableButtons(state);
        return result;
    });

extern "C" void hkMain() {
    gameSystemInit.installAtSym<"_ZN10GameSystem4initEv">();
    drawMainHook.installAtSym<"_ZN10GameSystem8drawMainEv">();
    fileDeviceMgrHook.installAtSym<"_ZN4sead13FileDeviceMgrC1Ev">();
    hakoniwaSequenceUpdate.installAtSym<"_ZN16HakoniwaSequence6updateEv">();

    btt::SettingsHooks::installSettingsHooks();

    hk::gfx::ImGuiBackendNvn::instance()->installHooks(false);

    DisableFullKeyState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_16NpadFullKeyStateEiRKj">();
    DisableHandheldState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_17NpadHandheldStateEiRKj">();
    DisableJoyDualState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_16NpadJoyDualStateEiRKj">();
    DisableJoyLeftState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_16NpadJoyLeftStateEiRKj">();
    DisableJoyRightState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_17NpadJoyRightStateEiRKj">();
}
