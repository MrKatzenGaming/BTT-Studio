#include "logger.h"

#include "hk/types.h"
#include <hk/diag/diag.h>
#include <hk/hook/Trampoline.h>

#include <sead/heap/seadDisposer.h>
#include <sead/heap/seadHeap.h>
#include <sead/heap/seadHeapMgr.h>
#include <sead/math/seadMathCalcCommon.h>
#include <sead/prim/seadEndian.h>

#include "game/Sequence/HakoniwaSequence.h"
#include <game/Sequence/ChangeStageInfo.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <nn/fs.h>
#include <nn/fs/fs_files.h>
#include <nn/fs/fs_types.h>
#include <nn/nifm.h>
#include <nn/socket.h>

#include "getHelper.h"

HkTrampoline<void> disableSocketInit = hk::hook::trampoline([]() -> void {});

static constexpr int socketPoolSize = 0x600000;
static constexpr int socketAllocPoolSize = 0x20000;
char socketPool[socketPoolSize + socketAllocPoolSize] __attribute__((aligned(0x1000)));

SEAD_SINGLETON_DISPOSER_IMPL(Logger);

void Logger::init(sead::Heap* heap) {
    mHeap = heap;
    sead::ScopedCurrentHeapSetter heapSetter(mHeap);

    al::FunctorV0M functor(this, &Logger::threadRecv);
    mRecvThread = new al::AsyncFunctorThread("Recv Thread", functor, 0, 0x20000, {});

    nn::nifm::Initialize();

    nn::socket::Initialize(socketPool, socketPoolSize, socketAllocPoolSize, 0xE);

    disableSocketInit.installAtSym<"_ZN2nn6socket10InitializeEPvmmi">();
}

s32 Logger::recvAll(u8* recvBuf, s32 remaining) {
    s32 totalReceived = 0;
    do {
        s32 recvLen = nn::socket::Recv(mSockFd, recvBuf, remaining, 0);
        if (recvLen <= 0) return recvLen;
        remaining -= recvLen;
        recvBuf += recvLen;
        totalReceived += recvLen;
    } while (remaining > 0);

    return totalReceived;
}

void Logger::threadRecv() {
    while (true) {
        handlePacket();
        nn::os::SleepThread(nn::TimeSpan::FromSeconds(1));
    }
}

void Logger::handlePacket() {
    u8 header[cPacketHeaderSize];
    recvAll(header, cPacketHeaderSize);
    RecPacketType packetType = RecPacketType(header[0]);

    switch (packetType) {
    case RecPacketType::None: {
        log(LogType::LogInfo, "received packet None");
        break;
    }
    case RecPacketType::Log: {
        u8 logType = header[1];
        u32 packetLen = header[4];

        log(LogType::LogInfo, "received packet Log: LogType = %d, len = %d (%d)", logType, packetLen);

        u8 chunkBuf[0x400];
        memset(chunkBuf, 0, sizeof(chunkBuf));
        s32 totalWritten = 0;
        while (totalWritten < packetLen) {
            s32 remaining = packetLen - totalWritten;
            s32 chunkLen = recvAll(chunkBuf, sead::Mathf::min(remaining, sizeof(chunkBuf)));
            totalWritten += chunkLen;
        }
        switch (logType) {
        case (int)LogType::LogInfo: log(LogType::LogInfo, "%s", chunkBuf); break;
        case (int)LogType::LogErr: log(LogType::LogErr, "%s", chunkBuf); break;
        case (int)LogType::LogWarn: log(LogType::LogWarn, "%s", chunkBuf); break;
        }
        break;
    }
    case RecPacketType::Warp: {
        u32 size = header[4];
        log(LogType::LogInfo, "received packet Warp: size = %d", size);

        u8 chunkBuf[0x400];
        memset(chunkBuf, 0, sizeof(chunkBuf));
        s32 totalWritten = 0;
        while (totalWritten < size) {
            s32 remaining = size - totalWritten;
            s32 chunkLen = recvAll(chunkBuf, sead::Mathf::min(remaining, sizeof(chunkBuf)));
            totalWritten += chunkLen;
        }
        HakoniwaSequence* gameSeq = helpers::tryGetHakoniwaSequence();
        if (!gameSeq) {
            log(LogType::LogErr, "not in HakoniwaSequence");
            break;
        };
        if (!gameSeq->mCurrentScene) {
            log(LogType::LogErr, "no scene");
            break;
        }

        if (chunkBuf[0] != '\0') {
            ChangeStageInfo stageInfo(
                gameSeq->mGameDataHolderAccessor.mData, "start", (char*)chunkBuf, false, -1, ChangeStageInfo::SubScenarioType::NO_SUB_SCENARIO
            );
            gameSeq->mGameDataHolderAccessor.mData->changeNextStage(&stageInfo, 0);
        } else {
            log(LogType::LogErr, "Invalid Stage");
        }
        break;
    }

    default: break;
    }
}

s32 Logger::connect(const char* serverIP, u16 port) {
    if (mState == State::CONNECTED) {
        log(LogType::LogErr, "Already connected, restarting connection");
        mState = State::DISCONNECTED;
        nn::socket::Close(mSockFd);
        return connect(serverIP, port);
    };

    in_addr hostAddress = { 0 };
    sockaddr_in serverAddr = { 0 };

    nn::nifm::SubmitNetworkRequest();

    while (nn::nifm::IsNetworkRequestOnHold()) {}

    if (!nn::nifm::IsNetworkAvailable()) {
        mState = State::UNAVAILABLE;
        return -1;
    }

    // create socket
    if ((mSockFd = nn::socket::Socket(AF_INET, SOCK_STREAM, 0)) < 0) return nn::socket::GetLastErrno();

    // configure server to connect to
    nn::socket::InetAton(serverIP, &hostAddress);
    serverAddr.sin_addr = hostAddress;
    serverAddr.sin_family = nn::socket::InetHtons(AF_INET);
    serverAddr.sin_port = nn::socket::InetHtons(port);

    // connect to server
    nn::Result result = nn::socket::Connect(mSockFd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result.IsFailure()) return nn::socket::GetLastErrno();

    mState = State::CONNECTED;

    hk::diag::debugLog("Connected to server %s:%d\n", serverIP, port);

    log(LogType::LogInfo, "connected!");
    log(LogType::LogWarn, "Logger warning test!");
    log(LogType::LogErr, "Logger error test!");

    mRecvThread->start();

    return 0;
}

void Logger::log(Logger::LogType type, const char* fmt, ...) {
    if (mState != Logger::State::CONNECTED) return;

    Packet packet;
    packet.type = PacketType::Log;

    va_list args;
    va_start(args, fmt);

    char buffer[0x400];

    switch (type) {
    case LogType::LogInfo: snprintf(buffer, sizeof(buffer), "[INFO] "); break;
    case LogType::LogErr: snprintf(buffer, sizeof(buffer), "[ERROR] "); break;
    case LogType::LogWarn: snprintf(buffer, sizeof(buffer), "[WARN] "); break;
    }

    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), fmt, args);
    va_end(args);

    snprintf((char*)packet.data, strlen(buffer) + 2, "%s\n", buffer);

    mMutex.lock();
    size_t packetSize = sizeof(packet.type) + std::min(strlen((char*)packet.data), sizeof(packet.data));
    nn::socket::Send(mSockFd, &packet, packetSize, 0);
    mMutex.unlock();
}

void Logger::log(Logger::LogType type, const char* fmt, va_list args) {
    if (mState != Logger::State::CONNECTED) return;

    Packet packet;
    packet.type = PacketType::Log;

    char buffer[0x400];

    switch (type) {
    case LogType::LogInfo: snprintf(buffer, sizeof(buffer), "[INFO] "); break;
    case LogType::LogErr: snprintf(buffer, sizeof(buffer), "[ERROR] "); break;
    case LogType::LogWarn: snprintf(buffer, sizeof(buffer), "[WARN] "); break;
    }

    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), fmt, args);

    snprintf((char*)packet.data, strlen(buffer) + 2, "%s\n", buffer);

    mMutex.lock();
    size_t packetSize = sizeof(packet.type) + std::min(strlen((char*)packet.data), sizeof(packet.data));
    nn::socket::Send(mSockFd, &packet, packetSize, 0);
    mMutex.unlock();
}
