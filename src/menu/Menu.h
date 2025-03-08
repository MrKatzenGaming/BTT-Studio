#pragma once

#include <heap/seadDisposer.h>

// #include "hk/gfx/DebugRenderer.h"
#include "sead/gfx/seadDrawContext.h"
#include "settings/SettingsMgr.h"

namespace btt {


class Menu {
    SEAD_SINGLETON_DISPOSER(Menu)

private:

    enum Page {
        Main = 0,
        Options = 1,
        Info = 2,
        Misc = 3
    };
    
    int mCurrentPage = Main;
    int mCurrentLine = 0;
    const char* mCharCursor = " ";
    sead::Heap* mHeap = nullptr;

    int currentStage = 0;
    int currentScenario = 0;
    int heldDirFrames = 0;

    SettingsMgr::Settings mSettings;

    void drawMain(void* renderer);
    void drawOptions(void* renderer);
    void drawInfo(void* renderer);
    void drawMisc(void* renderer);

public:
    Menu() = default;
    bool mIsEnabledInput = true;
    bool mIsEnabledMenu = true;

    void init(sead::Heap* heap);
    void draw(sead::DrawContext* drawContext);

};

} // namespace btt