#pragma once

#include <agl/common/aglDrawContext.h>

#include <sead/heap/seadHeap.h>

namespace btt::imgui {
typedef void (*ProcDrawFunc)();
void init(sead::Heap* heap);
void draw(agl::DrawContext* drawContext);
void addDrawFunc(ProcDrawFunc func);
void setScale();
void setupStyle();
} // namespace btt::imgui
