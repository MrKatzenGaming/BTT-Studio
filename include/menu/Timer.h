#pragma once

#include <sead/basis/seadTypes.h>

#include <nn/os.h>

namespace btt {

enum class TimerHookType : u8 {
    None,
    ShineGrab,
    ShineTick,
    GameStart,
    EscapeWire,
    CaptureGlow,
};

class Timer {
public:
    int mFrameTimer = 0;
    s64 mStartTick = 0;
    s64 mEndTick = 0;
    bool mIsRunning = false;
    s64 mShowSplitTick = 0;
    int mFrames = 0;

public:
    Timer();

    void start();
    void stop();
    void reset();
    void event(TimerHookType type);
    void showSplit();

    void draw();

    static Timer* sInstance;
};

} // namespace btt
