#pragma once

#include <stdint.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <algorithm>

#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#include <time.h>

// Замена gettimeofday для Windows
struct timeval {
    long tv_sec;
    long tv_usec;
};

inline int gettimeofday(struct timeval* tp, void* tzp) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t tt = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    tt /= 10;
    tt -= 11644473600000000ULL;
    tp->tv_sec = tt / 1000000;
    tp->tv_usec = tt % 1000000;
    return 0;
}

#define delay(ms) Sleep(ms)
#else
#include <sys/time.h>
#include <unistd.h>
#define delay(ms) usleep(ms * 1000)
#endif

#define DEVICE_NAME "testing"
#define RADIO_SX128X 1

// Фиктивные определения пинов для библиотеки SX1280
#define GPIO_PIN_BUSY UNDEF_PIN
#define GPIO_PIN_NSS_2 UNDEF_PIN
#define OPT_USE_HARDWARE_DCDC false

// Фиктивные определения пинов для библиотеки MSPVTX
#define OPT_HAS_VTX_SPI false
#define GPIO_PIN_SPI_VTX_NSS UNDEF_PIN

typedef uint8_t byte;

#define HEX 16
#define DEC 10

class Stream
{
public:
    Stream() {}
    virtual ~Stream() {}

    // Методы потока
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual void flush() = 0;

    // Методы печати
    virtual size_t write(uint8_t c) = 0;
    virtual size_t write(const uint8_t *s, size_t l) = 0;

    int print(const char *s) {return 0;}
    int print(uint8_t s) {return 0;}
    int print(uint8_t s, int radix) {return 0;}
    int println() {return 0;}
    int println(const char *s) {return 0;}
    int println(uint8_t s) {return 0;}
    int println(uint8_t s, int radix) {return 0;}
};

class HardwareSerial: public Stream {
public:
    // Методы потока
    int available() {return 0;}
    int read() {return -1;}
    int peek() {return 0;}
    void flush() {}
    void end() {}
    void begin(int baud) {}
    void enableHalfDuplexRx() {}
    int availableForWrite() {return 256;}

    // Методы печати
    size_t write(uint8_t c) {return 1;}
    size_t write(const uint8_t *s, size_t l) {return l;}

    int print(const char *s) {return 0;}
    int print(uint8_t s) {return 0;}
    int print(uint8_t s, int radix) {return 0;}
    int println() {return 0;}
    int println(const char *s) {return 0;}
    int println(uint8_t s) {return 0;}
    int println(uint8_t s, int radix) {return 0;}
};

static HardwareSerial Serial;
static Stream *SerialLogger = &Serial;

inline void interrupts() {}
inline void noInterrupts() {}

inline unsigned long micros() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

inline void delay(int32_t time) {
    usleep(time);
}

inline unsigned long millis() { return 0; }
inline void delayMicroseconds(int delay) { }
inline char *itoa(int32_t value, char *str, int base) { sprintf(str, "%d", value); return str; }
inline char *utoa(uint32_t value, char *str, int base) { sprintf(str, "%u", value); return str; }
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#ifdef _WIN32
#define random rand
#define srandom srand
#endif
