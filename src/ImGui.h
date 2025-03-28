#pragma once

#include "sead/heap/seadHeap.h"
#include "agl/common/aglDrawContext.h"

namespace btt::imgui {
    typedef void (*ProcDrawFunc)();
    void init(sead::Heap* heap);
    void draw(agl::DrawContext* drawContext);
    void addDrawFunc(ProcDrawFunc func);
    void setScale();
    void setupStyle();
}