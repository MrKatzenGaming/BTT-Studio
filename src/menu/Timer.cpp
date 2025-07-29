#include "menu/Timer.h"

#include <cstdio>

#include "nn/os.h"
#include "settings/SettingsMgr.h"

namespace btt {

Timer* Timer::sInstance = nullptr;

Timer::Timer() {
    sInstance = this;
}

void Timer::start() {
    mStartTick = nn::os::GetSystemTick().value;
    mIsRunning = true;
    mFrames = 0;
}

void Timer::stop() {
    if (mIsRunning) {
        mEndTick = nn::os::GetSystemTick().value;
        mIsRunning = false;
    }
}

void Timer::reset() {
    auto now = nn::os::GetSystemTick();
    mStartTick = now.value;
    mEndTick = now.value;
    mIsRunning = false;
    mFrames = 0;
}

void Timer::event(TimerHookType type) {
    bool isStartEvent = btt::SettingsMgr::instance()->getSettings()->mTimerStartType == type;
    bool isEndEvent = btt::SettingsMgr::instance()->getSettings()->mTimerEndType == type;
    if (!mIsRunning && isStartEvent)
        start();
    else if (btt::SettingsMgr::instance()->getSettings()->mIsEnableSegTimerSplit)
        showSplit();
    else if (mIsRunning && isEndEvent) {
        stop();
    }
}

void Timer::showSplit() {
    auto tick = nn::os::GetSystemTick();
    if (!mIsRunning) {
        mStartTick = tick.value;
        mEndTick = tick.value;
    }
    mShowSplitTick = tick.value;
}

void Timer::draw() {
    if (!btt::SettingsMgr::instance()->getSettings()->mIsEnableSegmentTimer) return;

    mFrames++;

    if (true) {
        char buffer[32] { 0 };

        s64 timerNow = mIsRunning ? nn::os::GetSystemTick().value : mEndTick;
        bool isShowSplit = mShowSplitTick && float(nn::os::GetSystemTick().value - mShowSplitTick) / nn::os::GetSystemTickFrequency().value <= 2;
        if (isShowSplit) timerNow = mShowSplitTick;

        s64 time = timerNow - mStartTick;
        s64 seconds = time / nn::os::GetSystemTickFrequency().value;
        s64 minutes = seconds / 60;
        s64 remainingSeconds = seconds % 60;
        s64 milliseconds = (time % nn::os::GetSystemTickFrequency().value) * 1000 / nn::os::GetSystemTickFrequency().value;

        if (time / nn::os::GetSystemTickFrequency().value >= 60)
            snprintf(buffer, 32, "%ld:%02ld.%03ld", minutes, remainingSeconds, milliseconds);
        else
            snprintf(buffer, 32, "%02ld.%03ld", remainingSeconds, milliseconds);

        if (isShowSplit)
            ImGui::GetForegroundDrawList()->AddText(
                ImGui::GetIO().Fonts->Fonts[0], 50, btt::SettingsMgr::instance()->getSettings()->mTimerPos, IM_COL32(255, 0, 0, mFrames % 20 <= 10 ? 255 : 0),
                buffer
            );
        else
            ImGui::GetForegroundDrawList()->AddText(
                ImGui::GetIO().Fonts->Fonts[0], 50, btt::SettingsMgr::instance()->getSettings()->mTimerPos, IM_COL32(255, 255, 255, 255), buffer
            );
    }
}

} // namespace btt
