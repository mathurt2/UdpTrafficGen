# UDP Traffic Generator (UdpTrafficGen)

## Project Overview

**Purpose:** Network testing tool that generates UDP packets at controlled constant bit rates (CBR) using a token bucket algorithm.

**Author:** Tushar Mathur  
**License:** BSD 3-Clause  
**Repository:** https://github.com/mathurt2/UdpTrafficGen

**Supported Platforms:**
- macOS 10.14.6+
- Ubuntu 18.04+ LTS
- Raspberry Pi OS (Raspberry Pi 3/4/5)

## Architecture

### Components

1. **udpCbrDriver.cpp** - Main driver program
   - Argument parsing and validation
   - Token bucket algorithm implementation
   - Timing control using microsecond precision
   - Main packet transmission loop

2. **udpCbr.c** - UDP socket operations (C)
   - Socket creation (`createSock`)
   - Packet payload generation (`udp_packet_gen`)
   - Packet transmission (`sendPkt`)

3. **udpCbrDriver.h** - Header file
   - External declarations for C functions

### Token Bucket Algorithm

The tool uses a token bucket algorithm for precise rate limiting:

```
tokenBucket += (cbr_mbps) × (time_delta_microseconds)
```

**Units:**
- `cbr`: Megabits per second (Mbps)
- `time_delta`: Microseconds (μs)
- `tokenBucket`: Bits (the 10^6 factors cancel out)
- Each packet consumes: `packet_size × 8` bits

**Rate Control:**
- Tokens accumulate based on target bit rate and elapsed time
- Packets are sent only when sufficient tokens are available
- Token bucket can go negative (debt mechanism for smooth rate limiting)
- 100μs sleep between iterations to smooth packet flow

## Usage

### Command Line Parameters

```bash
./udpCbrGen <duration> <dest_ip> <bitrate_mbps> <packet_size> <port>
```

**Parameters:**
1. **duration** - Length of run in seconds (minimum 15)
2. **dest_ip** - Destination IP address (A.B.C.D format)
3. **bitrate_mbps** - Average bit rate in Mbps (range: 10 - 500,000)
4. **packet_size** - IP PDU size in bytes (range: 1200 - 1500)
5. **port** - UDP port number (default: 9 if set to 0)

### Examples

```bash
# 100 Mbps for 60 seconds
./udpCbrGen 60 192.168.1.100 100 1500 9

# 10 Gbps for 5 minutes
./udpCbrGen 300 10.0.0.50 10000 1500 9

# Minimum and maximum rates
./udpCbrGen 15 127.0.0.1 10 1500 9      # 10 Mbps (minimum)
./udpCbrGen 15 127.0.0.1 500000 1500 9  # 500 Gbps (maximum)
```

### Default Parameters

If fewer than 5 arguments provided, defaults are used:
- Duration: 15 seconds
- Destination: 1.1.1.1
- Bit rate: 100 Mbps
- Packet size: 1500 bytes
- Port: 9

## Build Instructions

### Prerequisites

- gcc (for C compilation)
- g++ (for C++ compilation)
- make

### Compilation

```bash
# Clean previous builds
make clean

# Build the binary
make

# The output binary is 'udpCbrGen'
```

### Makefile Details

- C files compiled with `gcc`
- C++ files compiled with `g++`
- Debug flags: `-g3 -O0 -ggdb -Wall`
- No optimization (for debugging)

## Recent Changes

### Bug Fixes (2026-04-05)

**Pull Request:** [mathurt2/UdpTrafficGen#1](https://github.com/mathurt2/UdpTrafficGen/pull/1)

1. **Fixed Type Mismatch**
   - Changed `tokenBucket` from `unsigned long` to `long long` in header
   - Resolves undefined behavior from type inconsistency
   - Allows negative values for rate limiting debt mechanism

2. **Added Input Validation**
   - Maximum bit rate limit: 500 Gbps (500,000 Mbps)
   - Prevents integer overflow with unrealistic inputs
   - Clear error messages for out-of-range values

3. **Added Overflow Protection**
   - Pre-multiplication overflow check in token bucket calculation
   - Prevents undefined behavior with large bit rates and time deltas
   - Graceful error handling with diagnostic output

4. **Fixed Makefile**
   - Separate C and C++ compiler variables
   - Resolves linking errors on ARM64 (Apple Silicon)
   - Proper compilation of mixed C/C++ codebase

## Known Issues & Limitations

### Current Limitations

1. **Variable Length Array (VLA) Warning**
   - `unsigned char sendbuf[pktsize_payload]` uses Clang extension
   - Not standard C++, but widely supported
   - Could be replaced with `std::vector` for portability

2. **POSIX-Only**
   - Uses POSIX APIs: `sys/socket.h`, `arpa/inet.h`, `unistd.h`
   - Not compatible with Windows without Winsock2 port

3. **No Statistics**
   - No packet count, drop count, or bandwidth measurement
   - No real-time status output during run

4. **Fixed Sleep Interval**
   - Hardcoded 100μs sleep may not be optimal for all rates
   - Could be adaptive based on target bit rate

5. **No Error Recovery**
   - `sendPkt()` errors are logged but transmission continues
   - No distinction between transient (ENOBUFS) and fatal errors

### Compiler Warnings

```
udpCbrDriver.cpp:107:24: warning: variable length arrays in C++ are a Clang extension
```
- Safe to ignore on gcc/clang
- Consider replacing with `std::vector` for strict C++ compliance

## Future Enhancements

### Critical Priority (Ubuntu/Raspberry OS Robustness)

1. **Improved Timing Precision**
   - Replace `gettimeofday()` with `clock_gettime(CLOCK_MONOTONIC_RAW)`
   - Not affected by NTP adjustments
   - Better precision on embedded systems

2. **Socket Buffer Optimization**
   ```cpp
   int sndbuf = 1024 * 1024; // 1MB
   setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
   ```
   - Prevents packet drops at high rates on Raspberry Pi
   - Critical for rates > 100 Mbps

3. **Signal Handling**
   - Add SIGINT/SIGTERM handlers for graceful shutdown
   - Ensure socket is closed properly on Ctrl+C

4. **Error Handling Improvements**
   ```cpp
   if (errno == ENOBUFS || errno == EAGAIN) {
       // Transient error - skip packet
   } else {
       // Fatal error - report and exit
   }
   ```

### High Priority (Performance Optimization)

5. **Real-Time Scheduling**
   ```cpp
   sched_setscheduler(0, SCHED_FIFO, &param); // Requires root/CAP_SYS_NICE
   ```
   - Reduces timing jitter from 100s of μs to single digits
   - Critical for accurate rate limiting on Raspberry Pi

6. **CPU Affinity**
   ```cpp
   pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
   ```
   - Pin to specific CPU core
   - Reduces cache misses and timing variance
   - Especially important on Raspberry Pi 3/4/5 (multi-core)

7. **Memory Locking**
   ```cpp
   mlockall(MCL_CURRENT | MCL_FUTURE); // Requires root/CAP_IPC_LOCK
   ```
   - Prevents swapping on low-RAM systems (Raspberry Pi)
   - Eliminates huge latency spikes from page faults

8. **MTU Validation**
   ```cpp
   ioctl(sockfd, SIOCGIFMTU, &ifr);
   ```
   - Query interface MTU before sending
   - Prevent IP fragmentation
   - Important for WiFi (often < 1500 bytes)

### Medium Priority (Platform-Specific)

9. **Raspberry Pi Throttling Detection**
   - Monitor `/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq`
   - Warn if CPU is throttled due to temperature
   - Helps diagnose timing issues

10. **Systemd Service Integration**
    - Create `.service` file for automatic startup
    - Configure capabilities (CAP_NET_RAW, CAP_SYS_NICE, CAP_IPC_LOCK)
    - Production deployment on headless systems

11. **ARM Build Optimizations**
    ```makefile
    CFLAGS += -mcpu=cortex-a72 -mfpu=neon-fp-armv8  # Raspberry Pi 3/4
    ```

### Cross-Platform Portability

12. **Windows Compatibility**
    - Add Winsock2 support with `#ifdef _WIN32`
    - Abstract socket types (int vs SOCKET)
    - WSAStartup/WSACleanup initialization

13. **Modern C++ Refactoring**
    - Replace `gettimeofday()` with `std::chrono::high_resolution_clock`
    - Replace `usleep()` with `std::this_thread::sleep_for()`
    - Replace VLA with `std::vector<unsigned char>`
    - Use `uint64_t`/`int64_t` instead of `unsigned long long`/`long long`

14. **CMake Build System**
    - Cross-platform build configuration
    - Better dependency management
    - IDE integration (CLion, VS Code)

## Testing

### Build Verification

```bash
make clean && make
# Should compile without errors
# Warning about VLA is expected (non-critical)
```

### Functional Tests

```bash
# Normal operation
./udpCbrGen 15 127.0.0.1 100 1500 9
./udpCbrGen 30 127.0.0.1 1000 1500 9
./udpCbrGen 60 127.0.0.1 10000 1500 9

# Boundary conditions
./udpCbrGen 15 127.0.0.1 10 1500 9       # minimum valid
./udpCbrGen 15 127.0.0.1 500000 1500 9   # maximum valid

# Invalid inputs (should reject with error)
./udpCbrGen 15 127.0.0.1 5 1500 9        # too low
./udpCbrGen 15 127.0.0.1 1000000 1500 9  # too high
./udpCbrGen 10 127.0.0.1 100 1500 9      # duration too short
./udpCbrGen 15 127.0.0.1 100 1100 9      # packet too small
./udpCbrGen 15 127.0.0.1 100 1600 9      # packet too large
```

### Performance Testing (Ubuntu/Raspberry Pi)

```bash
# Monitor packet send errors
watch -n 1 'netstat -su | grep "packet send errors"'

# Test with CPU affinity (Ubuntu)
taskset -c 1 ./udpCbrGen 60 192.168.1.100 1000 1500 9

# Check Raspberry Pi throttling
vcgencmd get_throttled
vcgencmd measure_temp

# Monitor temperature during test
watch -n 1 'vcgencmd measure_temp && vcgencmd measure_clock arm'
```

### Network Validation

```bash
# On receiving system, use tcpdump to verify rate
sudo tcpdump -i eth0 -n udp port 9 -c 1000

# Or use iperf3 server mode for comparison
iperf3 -s -p 9
```

## Development Guidelines

### Code Style

- C code: K&R style with 4-space indentation
- C++ code: Mix of K&R and modern C++ conventions
- Tabs used in some places (Makefile)
- Line comments with `//`, block comments with `/* */`

### Commit Messages

Use conventional commit format:
```
<type>: <subject>

<body>

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

Types: `fix`, `feat`, `refactor`, `docs`, `test`, `build`

### Testing Requirements

- Code must compile without errors on Ubuntu and macOS
- Test at minimum, maximum, and typical bit rates
- Verify input validation rejects invalid parameters
- Run extended tests (5+ minutes) to check for overflow/drift

### Pull Request Guidelines

- Target upstream repository: `mathurt2/UdpTrafficGen`
- Include clear description of changes
- Reference related issues if applicable
- Test on both x86_64 and ARM if possible

## Troubleshooting

### Compilation Issues

**Error: Undefined symbols for architecture arm64**
- Solution: Use separate gcc (C) and g++ (C++) compilers
- Fixed in current Makefile

**Error: Permission denied when binding socket**
- Solution: Use port > 1024 or run with sudo
- Default port 9 is privileged (<1024)

### Runtime Issues

**Packet send errors (ENOBUFS)**
- Increase socket send buffer size
- Reduce bit rate
- Add sleep between packets

**High timing jitter**
- Run with real-time priority (sudo)
- Pin to CPU core with taskset
- Check for thermal throttling (Raspberry Pi)

**Raspberry Pi overheating**
- Add heatsink or fan
- Reduce bit rate
- Increase sleep interval
- Monitor with `vcgencmd measure_temp`

## References

### Documentation

- Token Bucket Algorithm: https://en.wikipedia.org/wiki/Token_bucket
- UDP Protocol: RFC 768
- IPv4 Header: RFC 791

### Related Tools

- iperf3: Network performance measurement
- tcpdump: Packet capture and analysis
- netstat: Network statistics

### Platform Information

- Raspberry Pi Documentation: https://www.raspberrypi.com/documentation/
- Linux `sched_setscheduler`: man 2 sched_setscheduler
- Linux Socket Options: man 7 socket

---

**Last Updated:** 2026-04-05
**Version:** 1.1 (with overflow bug fixes)
