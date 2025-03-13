#include "SettingsMgr.h"

#include <heap/seadHeapMgr.h>
#include "helpers/fsHelper.h"
#include "hk/diag/diag.h"

namespace btt {
    SEAD_SINGLETON_DISPOSER_IMPL(SettingsMgr);

    void SettingsMgr::saveSettings() {
        Settings* mnSettings = &mSettings;
        if (!FsHelper::isDirExist("sd:/BTT-Studio")) {
            nn::fs::CreateDirectory("sd:/BTT-Studio");
        }

        FsHelper::writeFileToPath(reinterpret_cast<void*>(mnSettings), sizeof(Settings), mPath);
    }

    void SettingsMgr::loadSettings() {
        if (!FsHelper::isFileExist(mPath)) {
            saveSettings();
            return;
        }

        FsHelper::LoadData data;
        data.path = mPath;
        FsHelper::loadFileFromPath(data);

        if (data.buffer != nullptr && data.bufSize == sizeof(Settings)) {
            Settings* configData = reinterpret_cast<Settings*>(data.buffer);
            mSettings = *configData;
        }
    }


} // namespace btt