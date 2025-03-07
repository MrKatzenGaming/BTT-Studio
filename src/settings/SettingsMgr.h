#pragma once

#include <heap/seadDisposer.h>

namespace btt {

class SettingsMgr {
    SEAD_SINGLETON_DISPOSER(SettingsMgr)

private:
    sead::Heap* mHeap;
public:
    SettingsMgr() = default;

    struct Settings {
        bool mIsEnableMoonJump = false;
    } mSettings;

    void init(sead::Heap* heap);
    };


} // namespace btt