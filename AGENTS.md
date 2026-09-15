# AGENTS.md - RTSP Server Development Guide

## Project Overview

**Language:** C++11  
**Build System:** CMake  
**Platforms:** x86 (local), ARM (cross-compilation)  
**Purpose:** RTSP streaming server for H.264/H.265 video and AAC audio

---

## Build Commands

### Local Build (x86)

```bash
mkdir -p build && cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=../arm_install -DSYNC_MODE=AV_SYNC_AUDIO_MASTER
make -j16
make install
```

### ARM Cross-Compilation

```bash
mkdir -p arm_build && cd arm_build
cmake ../ -DCMAKE_INSTALL_PREFIX=../arm_install -DSYNC_MODE=AV_SYNC_AUDIO_MASTER \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/arm-toolchain.cmake
make -j16
make install
```

### Build Options

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `SYNC_MODE` | `AV_SYNC_AUDIO_MASTER`, `AV_SYNC_VIDEO_MASTER`, `AV_SYNC_EXTERNAL_CLOCK` | `AV_SYNC_AUDIO_MASTER` | A/V sync mode |
| `CMAKE_INSTALL_PREFIX` | Path | `./arm_install` | Installation directory |

### Quick Build Script

```bash
./run.sh  # Runs cmake and make in build directory
```

---

## Code Style Guidelines

### General Rules

- **C++ Standard:** C++11 only (no C++14/17/20 features)
- **Indentation:** 4 spaces (no tabs)
- **Line Length:** Keep lines under 120 characters

### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | PascalCase | `RTSP_S`, `ClientManager` |
| Member variables | camelCase or `_prefix` | `clientRtpPort`, `_f_list` |
| Methods/Functions | camelCase or snake_case | `mutex_lock()`, `get_video_frame()` |
| Constants/Macros | UPPER_SNAKE_CASE | `SERVERRTPPORT`, `MAX_RECV_BUF_LEN` |
| Type definitions | PascalCase | `RtspMethodT`, `RtpStream` |
| Struct members | camelCase | `sync_byte`, `transport_error_indicator` |

### Brace Style (K&R)

```cpp
if (condition) {
    // code
} else {
    // code
}

while (condition) {
    // code
}

for (init; condition; increment) {
    // code
}
```

### Header Guards

```cpp
#ifndef _RTSPSERVER_HPP_
#define _RTSPSERVER_HPP_
// ... content ...
#endif
```

### Class Structure

```cpp
class MyClass : public BaseClass {
public:
    MyClass() {};
    ~MyClass() { /* cleanup */ };
    
    void publicMethod();
    
private:
    int _privateMember;
    void privateMethod();
};
```

### Comments

- Chinese comments for function descriptions (per project convention):
```cpp
/*
 *函数描述：添加播放源
 *参数：
 *   sourceName：rtsp的播放地址
 */
```
- English for inline technical comments

---

## Project Structure

```
rtspServer-main/
├── main.cpp              # Entry point, demo with test media
├── rtspserver.hpp/cpp    # RTSP server class
├── clientsession.hpp/cpp # Client session handling
├── clientManager.hpp/cpp # Media source management (singleton)
├── client_listen.hpp/cpp # Client connection monitoring
├── sock.hpp/cpp          # Socket operations
├── thread_rtsp.hpp/cpp   # Thread base class
├── r_media_stream.hpp/cpp# RTP/RTCP media streaming
├── sdp.hpp/cpp           # SDP (Session Description Protocol)
├── Def.h/cpp             # Definitions, enums, structures
├── mutex.hpp/cpp         # Mutex wrapper
├── ts_decode.hpp/cpp     # MPEG-TS demultiplexing
├── en_de_ts/             # TS encoding/decoding
│   ├── ts.hpp/cpp, pat.hpp/cpp, pmt.hpp/cpp, pes.hpp/cpp
│   ├── PCR.hpp/cpp, FileStream.hpp/cpp
│   └── ts_encoder.hpp/cpp, ts_decoder.hpp/cpp
├── fifo/                 # Thread-safe FIFO buffers
├── ring_buffer/          # Ring buffer implementation
└── time_base/            # Timestamp utilities
```

---

## Architecture Patterns

### Singleton Pattern

```cpp
class ClientManager {
public:
    static ClientManager& GetInstance() {
        static ClientManager instance;
        return instance;
    }
    ClientManager(const ClientManager&) = delete;
    ClientManager &operator=(const ClientManager &) = delete;
};
```

### Thread Base Class

```cpp
class THREAD_RTSP {
public:
    bool start();
    virtual void* thread_proc() = 0;
};
```

### Media Source Callbacks

```cpp
typedef int (*read_video)(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts);
typedef int (*read_audio)(void *opaque, uint8_t *data, int len, uint64_t *pts);
```

### RTSP Protocol Methods

Supported: OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN, GET_PARAMETER, SET_PARAMETER

---

## Important Notes

1. **No External Dependencies** - This is a self-contained implementation using only standard C++ and POSIX libraries
2. **Thread Safety** - Use the `MUTEX_RTSP` class for mutex operations
3. **Memory Management** - Mix of raw pointers; always pair `new`/`delete` and `new[]`/`delete[]`
4. **Socket Programming** - Use the `SOCK_T` base class for network operations
5. **Media URLs** - Format: `rtsp://localip:8554/sourcename`
