#include "saveFileHelper.h"

#include <sead/heap/seadHeapMgr.h>

#include <cstddef>
#include <nn/fs.h>

#include "fsHelper.h"
#include "logger.h"
#include "os.h"
#include "settings/SettingsMgr.h"

SEAD_SINGLETON_DISPOSER_IMPL(SaveFileHelper);

void SaveFileHelper::ThreadSave() {
    while (true) {
        SaveFileHelper::instance()->saveSettings();
        nn::os::SleepThread(nn::TimeSpan::FromSeconds(60));
    }
}

void SaveFileHelper::init(sead::Heap* heap) {
    mHeap = heap;
    sead::ScopedCurrentHeapSetter heapSetter(mHeap);

    al::FunctorV0M functor(this, &SaveFileHelper::ThreadSave);
    mSaveThread = new al::AsyncFunctorThread("Save Thread", functor, 0, 0x20000, {});
}

void SaveFileHelper::saveSettings() {
    if (!FsHelper::isDirExist("sd:/BTT-Studio")) {
        nn::fs::CreateDirectory("sd:/BTT-Studio");
    }

    FsHelper::writeFileToPath(reinterpret_cast<void*>(&btt::SettingsMgr::instance()->mSettings), sizeof(btt::SettingsMgr::Settings), mSettingsPath);
}

void SaveFileHelper::loadSettings(sead::Heap* heap) {
    if (!FsHelper::isFileExist(mSettingsPath)) {
        saveSettings();
        return;
    }

    FsHelper::LoadData data;
    data.path = mSettingsPath;
    FsHelper::loadFileFromPath(data, heap);

    if (!data.buffer) {
        Logger::instance()->log(Logger::LogType::LogErr, "Failed to load settings from %s", mSettingsPath);
        return;
    }
    if (data.bufSize != sizeof(btt::SettingsMgr::Settings)) {
        Logger::instance()->log(Logger::LogType::LogErr, "Invalid settings size: %zu, expected: %zu", data.bufSize, sizeof(btt::SettingsMgr::Settings));
        return;
    }
    btt::SettingsMgr::Settings* configData = reinterpret_cast<btt::SettingsMgr::Settings*>(data.buffer);

    if (strcmp(configData->Version, btt::SettingsMgr::instance()->mSettings.Version) != 0) {
        Logger::instance()->log(
            Logger::LogType::LogWarn, "Version mismatch: %s != %s, resetting Settings", configData->Version, btt::SettingsMgr::instance()->mSettings.Version
        );
        return;
    }

    btt::SettingsMgr::instance()->mSettings = *configData;
}

void SaveFileHelper::saveTeleport(btt::Menu::TpState* states, size_t count) {
    if (!FsHelper::isDirExist("sd:/BTT-Studio")) {
        nn::fs::CreateDirectory("sd:/BTT-Studio");
    }

    FsHelper::writeFileToPath(reinterpret_cast<void*>(states), sizeof(btt::Menu::TpState) * count, mtpPath);
}

void SaveFileHelper::loadTeleport(btt::Menu::TpState* states, size_t count, sead::Heap* heap) {
    if (!FsHelper::isFileExist(mtpPath)) {
        saveTeleport(states, count);
        return;
    }

    FsHelper::LoadData data;
    data.path = mtpPath;
    FsHelper::loadFileFromPath(data, heap);

    if (data.buffer != nullptr && data.bufSize == sizeof(btt::Menu::TpState) * count) {
        btt::Menu::TpState* loadedStates = reinterpret_cast<btt::Menu::TpState*>(data.buffer);
        for (size_t i = 0; i < count; ++i) {
            states[i] = loadedStates[i];
        }
    }
}
