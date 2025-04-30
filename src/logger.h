#include "al/Library/Thread/AsyncFunctorThread.h"

#include <sead/basis/seadTypes.h>
#include <sead/heap/seadExpHeap.h>

#include <nn/os.h>

class Logger {
    SEAD_SINGLETON_DISPOSER(Logger);

public:
    Logger() = default;

    void init(sead::Heap* heap);
    s32 connect(const char* serverIP, u16 port);

    enum class SendPacketType : u8 {
        LogInfo = 0x01,
        LogErr = 0x02,
        LogWarn = 0x03,
    };

    void log(Logger::SendPacketType type, const char* fmt, ...);
    void log(Logger::SendPacketType type, const char* fmt, va_list args);

private:
    enum class State {
        UNINITIALIZED,
        DISCONNECTED,
        CONNECTED,
    };

    enum class RecPacketType : u8 {
        None = 0x00,
        Script = 0x01,
    };

    struct Packet {
        SendPacketType type;
        u8 data[0x400];
    };

    constexpr static s32 cPacketHeaderSize = 0x10;

    sead::Heap* mHeap = nullptr;
    al::AsyncFunctorThread* mRecvThread = nullptr;
    void* mThreadStack = nullptr;
    s32 mSockFd = -1;
    State mState = State::UNINITIALIZED;
    RecPacketType mCurPacketType = RecPacketType::None;
    Packet* mCurPacket = nullptr;

    void threadRecv();
    void handlePacket();
    s32 recvAll(u8* recvBuf, s32 remaining);
};
