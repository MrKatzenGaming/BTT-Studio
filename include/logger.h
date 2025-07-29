#include "hk/os/Mutex.h"

#include "al/Library/Thread/AsyncFunctorThread.h"

#include <sead/basis/seadTypes.h>
#include <sead/heap/seadExpHeap.h>

#include <nn/os.h>
#include <queue>

class Logger {
    SEAD_SINGLETON_DISPOSER(Logger);

public:
    Logger() = default;

    void init(sead::Heap* heap);
    s32 connect(const char* serverIP, u16 port);

    enum class LogType : u8 {
        LogInfo = 0x01,
        LogErr = 0x02,
        LogWarn = 0x03,
    };

    void log(LogType type, const char* fmt, ...);
    void log(LogType type, const char* fmt, va_list args);

private:
    enum class State {
        UNINITIALIZED,
        UNAVAILABLE,
        DISCONNECTED,
        CONNECTED,
    };

    enum class RecPacketType : u8 {
        None = 0x00,
        Log = 0x01,
        Warp = 0x02,
    };

    enum class PacketType : u8 {
        Log = 0x01,
    };

    struct Packet {
        PacketType type;
        u8 data[0x400];
    };

    constexpr static s32 cPacketHeaderSize = 0x10;

    sead::Heap* mHeap = nullptr;
    al::AsyncFunctorThread* mRecvThread = nullptr;
    void* mThreadStack = nullptr;
    s32 mSockFd = -1;
    State mState = State::UNINITIALIZED;
    RecPacketType mCurPacketType = RecPacketType::None;
    void threadRecv();
    void handlePacket();
    s32 recvAll(u8* recvBuf, s32 remaining);
    void threadSend();
    std::queue<Packet> packetQueue;
    hk::os::Mutex mMutex;
};
