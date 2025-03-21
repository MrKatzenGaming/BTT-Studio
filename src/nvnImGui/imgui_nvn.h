#pragma once

#include "nn/types.h"
#include "imgui.h"

#define IMGUINVN_ADDDRAW(FuncBody) nvnImGui::addDrawFunc([]() {             \
FuncBody                                                                    \
});                                                                         \

namespace nvnImGui {

    typedef void (*ProcDrawFunc)();

    void InstallHooks();

    bool InitImGui();

    void procDraw();

    void addDrawFunc(ProcDrawFunc func);
}
