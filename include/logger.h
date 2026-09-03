#pragma once

#include "sead/heap/seadDisposer.h"

class Logger {
    SEAD_SINGLETON_DISPOSER(Logger);

public:
    Logger() = default;

    void init(sead::Heap* heap);

    enum class LogType : u8 {
        LogInfo = 0x01,
        LogErr = 0x02,
        LogWarn = 0x03,
    };

    void log(const char* fmt, ...);
    void log(LogType type, const char* fmt, ...);
    void log(LogType type, const char* fmt, va_list args);
};
