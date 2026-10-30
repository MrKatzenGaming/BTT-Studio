#include "logger.h"

#include "hk/diag/diag.h"
#include "hk/diag/ipclogger.h"

#include "sead/heap/seadDisposer.h"
#include "sead/heap/seadHeap.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

SEAD_SINGLETON_DISPOSER_IMPL(Logger);

void Logger::init(sead::Heap* heap) {
    hk::diag::ipclogger::IpcLogger::initialize();

    hk::diag::logLine("\n\n\nBTT-Stuido --- Logger Initialized");
}

void Logger::log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(LogType::LogInfo, fmt, args);
    va_end(args);
}

void Logger::log(Logger::LogType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[0x400];

    switch (type) {
    case LogType::LogInfo: snprintf(buffer, sizeof(buffer), "\x1b[1;32m[INFO] \x1b[0m"); break;
    case LogType::LogErr: snprintf(buffer, sizeof(buffer), "\x1b[1;31m[ERROR] \x1b[0m"); break;
    case LogType::LogWarn: snprintf(buffer, sizeof(buffer), "\x1b[1;33m[WARN] \x1b[0m"); break;
    }

    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), fmt, args);
    va_end(args);

    hk::diag::logLine("%s", buffer);
}

void Logger::log(Logger::LogType type, const char* fmt, va_list args) {
    char buffer[0x400];

    switch (type) {
    case LogType::LogInfo: snprintf(buffer, sizeof(buffer), "[INFO] "); break;
    case LogType::LogErr: snprintf(buffer, sizeof(buffer), "[ERROR] "); break;
    case LogType::LogWarn: snprintf(buffer, sizeof(buffer), "[WARN] "); break;
    }

    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), fmt, args);

    hk::diag::logLine("%s", buffer);
}
