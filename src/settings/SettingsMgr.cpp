#include "SettingsMgr.h"

#include <heap/seadHeapMgr.h>

namespace btt {
    SEAD_SINGLETON_DISPOSER_IMPL(SettingsMgr);

    void SettingsMgr::init(sead::Heap* heap) {
        mHeap = heap;
        sead::ScopedCurrentHeapSetter heapSetter(mHeap);
    }

} // namespace btt