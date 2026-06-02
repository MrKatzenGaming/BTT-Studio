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

#include "customBoot/BootHooks.hpp"
#include "helpers/InputHelper.h"
#include "helpers/saveFileHelper.h"
#include "ImGui.h"
#include "logger.h"
#include "menu/InputDisplay.h"
#include "menu/Menu.h"
#include "menu/Timer.h"
#include "settings/SettingsHooks.h"
#include "settings/TimerHooks.h"

using namespace hk;

namespace btt {
sead::Heap* initializeHeap() {
    return sead::ExpHeap::create(2_MB, "BTTStudioHeap", al::getStationedHeap(), 8, sead::Heap::cHeapDirection_Forward, false);
}
} // namespace btt

HkTrampoline gameSystemInit = [](TrampolineStatic(), GameSystem* gameSystem) -> void {
    sead::Heap* heap = btt::initializeHeap();

    Logger* logger = Logger::createInstance(heap);
    logger->init(heap);

#ifdef BTTDEBUG
    logger->connect("192.168.178.41", 8171);
#endif

    btt::SettingsMgr::createInstance(heap);
    logger->log(Logger::LogType::LogInfo, "SettingsMgr instance created");
    btt::Menu* menu = btt::Menu::createInstance(heap);
    logger->log(Logger::LogType::LogInfo, "Menu instance created");
    btt::Timer* timer = new btt::Timer();
    logger->log(Logger::LogType::LogInfo, "Timer instance created");
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
            if (btt::SettingsMgr::instance()->getSettings()->mIsEnableSegmentTimer) {
                btt::Timer* timer = btt::Timer::sInstance;
                if (timer) {
                    timer->draw();
                }
            }
            menu->drawPopup();
        }
    });
    logger->log(Logger::LogType::LogInfo, "ImGui setup done");

    InputHelper::setDisableMouse(true);

    orig(gameSystem);

    logger->log(Logger::LogType::LogInfo, "GameSystem initialized");
};

HkTrampoline drawMainHook = [](TrampolineStatic(), GameSystem* gameSystem) -> void {
    orig(gameSystem);

    auto drawContext = Application::instance()->mDrawSystemInfo->drawContext;
    btt::imgui::draw(drawContext);
};

HkTrampoline fileDeviceMgrHook = [](TrampolineStatic(), sead::FileDeviceMgr* fileDeviceMgr) -> void {
    orig(fileDeviceMgr);

    fileDeviceMgr->mMountedSd = nn::fs::MountSdCardForDebug("sd") == 0;
};

int timer = 0;
HkTrampoline hakoniwaSequenceUpdate = [](TrampolineStatic(), HakoniwaSequence* hakoniwaSequence) -> void {
    orig(hakoniwaSequence);

    btt::Menu::instance()->handleAlways();
};

void disableButtons(nn::hid::NpadBaseState* state) {
    if (!InputHelper::isReadInputs() && InputHelper::isInputToggled() && btt::Menu::instance()->mIsEnabledMenu) {
        // clear out the data within the state (except for the sampling number and attributes)
        state->mButtons = nn::hid::NpadButtonSet();
        state->mAnalogStickL = nn::hid::AnalogStickState();
        state->mAnalogStickR = nn::hid::AnalogStickState();
    }
}

HkTrampoline DisableFullKeyState = [](TrampolineStatic(), int* unkInt, nn::hid::NpadFullKeyState* state, int count, const unsigned int& port) -> int {
    int result = orig(unkInt, state, count, port);
    disableButtons(state);
    return result;
};

HkTrampoline DisableHandheldState = [](TrampolineStatic(), int* unkInt, nn::hid::NpadHandheldState* state, int count, const unsigned int& port) -> int {
    int result = orig(unkInt, state, count, port);
    disableButtons(state);
    return result;
};

HkTrampoline DisableJoyDualState = [](TrampolineStatic(), int* unkInt, nn::hid::NpadJoyDualState* state, int count, const unsigned int& port) -> int {
    int result = orig(unkInt, state, count, port);
    disableButtons(state);
    return result;
};

HkTrampoline DisableJoyLeftState = [](TrampolineStatic(), int* unkInt, nn::hid::NpadJoyLeftState* state, int count, const unsigned int& port) -> int {
    int result = orig(unkInt, state, count, port);
    disableButtons(state);
    return result;
};

HkTrampoline DisableJoyRightState = [](TrampolineStatic(), int* unkInt, nn::hid::NpadJoyRightState* state, int count, const unsigned int& port) -> int {
    int result = orig(unkInt, state, count, port);
    disableButtons(state);
    return result;
};

extern "C" void hkMain() {
    gameSystemInit.installAtSym<"_ZN10GameSystem4initEv">();
    drawMainHook.installAtSym<"_ZN10GameSystem8drawMainEv">();
    fileDeviceMgrHook.installAtSym<"_ZN4sead13FileDeviceMgrC1Ev">();
    hakoniwaSequenceUpdate.installAtSym<"_ZN16HakoniwaSequence6updateEv">();

    btt::SettingsHooks::installSettingsHooks();
    btt::TimerHooks::installTimerHooks();

    customboot::prepareCustomBootHook.installAtSym<"_ZN10BootLayoutC1ERKN2al14LayoutInitInfoE">();

    hk::gfx::ImGuiBackendNvn::instance()->installHooks(false);

    DisableFullKeyState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_16NpadFullKeyStateEiRKj">();
    DisableHandheldState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_17NpadHandheldStateEiRKj">();
    DisableJoyDualState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_16NpadJoyDualStateEiRKj">();
    DisableJoyLeftState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_16NpadJoyLeftStateEiRKj">();
    DisableJoyRightState.installAtSym<"_ZN2nn3hid6detail13GetNpadStatesEPiPNS0_17NpadJoyRightStateEiRKj">();
}
