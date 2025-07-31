#pragma once
#include "hk/hook/Trampoline.h"

#include "al/Library/Base/StringUtil.h"
#include "al/Library/Layout/LayoutInitInfo.h"

#include "game/Sequence/HakoniwaSequence.h"

#include <typeinfo>

#include "CustomBootLoad.hpp"
#include "logger.h"
#include "nn/oe.h"

namespace customboot {
static float autoCloseAfter = nn::oe::GetOperationMode() == nn::oe::OperationMode_Docked ? 13.f : 13.f;

static al::LayoutInitInfo copiedInitInfo;

static HkTrampoline<void, BootLayout*, al::LayoutInitInfo&> prepareCustomBootHook =
    hk::hook::trampoline([](BootLayout* boot, al::LayoutInitInfo& initInfo) -> void {
        al::NerveExecutor* e;
        __asm("MOV %[result], X19" : [result] "=r"(e)); // Hacky but it works

        if (al::isEqualString(typeid(*e).name(), typeid(HakoniwaSequence).name())) {
            new CustomBootLoad(((HakoniwaSequence*)e)->mResourceLoader, initInfo, autoCloseAfter);
            Logger::instance()->log(Logger::LogType::LogInfo, "CustomBoot created");
        }

        prepareCustomBootHook.orig(boot, initInfo);
    });

} // namespace customboot
