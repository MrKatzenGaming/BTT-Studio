#pragma once

#include <heap/seadDisposer.h>

#include "hk/gfx/DebugRenderer.h"
#include "sead/gfx/seadDrawContext.h"

#define printf(FORMAT, ...) renderer->printf(FORMAT, ##__VA_ARGS__)
#define TEXT(LINE, FMT, ...) renderer->setCursor({20, 370 + (LINE+1) * 20}); printf(FMT, ##__VA_ARGS__)

#define CURSOR(LINE) mCharCursor = mCurrentLine == LINE ? ">" : " "; renderer->setCursor({20, 370 + (LINE+1) * 20})

#define TITLE(NAME) printf(NAME "%s\n", mIsEnabledInput ? "" : " [Menu disabled]")

#define CHANGE_PAGE(NAME, PAGE, LINE) CURSOR(LINE);\
    printf("%s" NAME "\n", mCharCursor);\
    if (mIsEnabledInput && mCurrentLine == LINE && al::isPadTriggerRight(-1)) \
        { mCurrentPage = PAGE; mCurrentLine = 0;}

#define BACK_PAGE(PAGE, LINE) CURSOR(LINE);\
    printf("%sBack\n", mCharCursor);\
    if (mIsEnabledInput && mCurrentLine == LINE && (al::isPadTriggerRight(-1) ||  al::isPadTriggerLeft(-1))) \
        { mCurrentPage = PAGE; mCurrentLine = 0;}

// Sets max amount of lines for a page
#define MAX_LINE(LINE) if (mCurrentLine >= LINE) mCurrentLine = 0;\
        else if (mCurrentLine < 0) mCurrentLine = LINE - 1

#define TOGGLES(BOOL, LINE) CURSOR(LINE);\
        if (mIsEnabledInput && mCurrentLine == LINE && (al::isPadTriggerLeft(-1) || al::isPadTriggerRight(-1))) BOOL = !BOOL;

#define TOGGLE(NAME, BOOL, LINE) TOGGLES(BOOL, LINE);\
        printf("%s" NAME ": %s\n", mCharCursor, BOOL ? "Enabled" : "Disabled")

#define TRIGGER(NAME, LINE, ACTION) CURSOR(LINE);\
        printf("%s" NAME "\n", mCharCursor);\
        if (mIsEnabledInput  && mCurrentLine == LINE  && al::isPadTriggerRight(-1)) {ACTION;}

#define TRIGGER_FMT(NAME, LINE, ACTION, FMT...) CURSOR(LINE);\
        printf("%s" NAME "\n", mCharCursor, FMT);\
        if (mIsEnabledInput && mCurrentLine == LINE  && al::isPadTriggerRight(-1)) {ACTION;}

#define MAX_MENU_PAGES 3

namespace btt {


class Menu {
    SEAD_SINGLETON_DISPOSER(Menu)

private:

    enum Page {
        Main = 0,
        Options = 1,
        Info = 2
    };
    
    int mCurrentPage = Main;
    int mCurrentLine = 0;
    const char* mCharCursor = " ";
    sead::Heap* mHeap = nullptr;

    void drawMain(hk::gfx::DebugRenderer* renderer);
    void drawOptions(hk::gfx::DebugRenderer* renderer);
    void drawInfo(hk::gfx::DebugRenderer* renderer);

public:
    Menu() = default;
    bool mIsEnabledInput = true;

    void init(sead::Heap* heap);
    void draw(sead::DrawContext* drawContext);

};

} // namespace btt