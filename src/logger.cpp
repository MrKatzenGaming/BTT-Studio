#include "logger.h"

#include "hk/types.h"
#include <hk/diag/diag.h>
#include <hk/hook/Trampoline.h>

#include <sead/heap/seadDisposer.h>
#include <sead/heap/seadHeap.h>
#include <sead/heap/seadHeapMgr.h>
#include <sead/math/seadMathCalcCommon.h>
#include <sead/prim/seadEndian.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <nn/fs.h>
#include <nn/fs/fs_files.h>
#include <nn/fs/fs_types.h>
#include <nn/socket.h>

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

    nn::socket::Initialize(socketPool, socketPoolSize, socketAllocPoolSize, 0xE);

    mCurPacket = new Packet;

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
    case RecPacketType::Script: {
        break;
    }
    case RecPacketType::None:
    default: break;
    }
}

s32 Logger::connect(const char* serverIP, u16 port) {
    if (mState == State::CONNECTED) return 0;

    in_addr hostAddress = { 0 };
    sockaddr_in serverAddr = { 0 };

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

    log(SendPacketType::LogInfo, "connected!");

    mRecvThread->start();

    return 0;
}

void Logger::log(Logger::SendPacketType type, const char* fmt, ...) {
    if (mState != Logger::State::CONNECTED) return;

    memset(mCurPacket, 0, sizeof(Packet));

    mCurPacket->type = type;

    va_list args;
    va_start(args, fmt);

    // Format the message into the packet's data field
    int formattedLength = vsnprintf((char*)mCurPacket->data, sizeof(mCurPacket->data), fmt, args);
    va_end(args);

    // Calculate the actual size of the packet to send
    size_t packetSize = sizeof(mCurPacket->type) + (formattedLength < 0 ? 0 : std::min((size_t)formattedLength, sizeof(mCurPacket->data)));

    // Send the packet
    s32 r = nn::socket::Send(mSockFd, mCurPacket, packetSize, 0);
}

void Logger::log(Logger::SendPacketType type, const char* fmt, va_list args) {
    if (mState != Logger::State::CONNECTED) return;

    memset(mCurPacket, 0, sizeof(Packet));

    mCurPacket->type = type;

    // Format the message into the packet's data field
    int formattedLength = vsnprintf((char*)mCurPacket->data, sizeof(mCurPacket->data), fmt, args);

    // Calculate the actual size of the packet to send
    size_t packetSize = sizeof(mCurPacket->type) + (formattedLength < 0 ? 0 : std::min((size_t)formattedLength, sizeof(mCurPacket->data)));

    // Send the packet
    s32 r = nn::socket::Send(mSockFd, mCurPacket, packetSize + 1, 0);
}
