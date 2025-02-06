#pragma once
#if !defined TARGET_NATIVE
#include <Arduino.h>
#endif

#define UNDEF_PIN (-1)

/// General Features ///
#define LED_MAX_BRIGHTNESS 50 //0..255 for max led brightness

/////////////////////////

#define WORD_ALIGNED_ATTR __attribute__((aligned(4)))
#define WORD_PADDED(size) (((size)+3) & ~3)

#ifdef PLATFORM_STM32
/* ICACHE_RAM_ATTR1 is always linked into RAM */
#define ICACHE_RAM_ATTR1  __section(".ram_code")
/* ICACHE_RAM_ATTR2 is linked into RAM only if enough space */
#if RAM_CODE_LIMITED
#define ICACHE_RAM_ATTR2
#else
#define ICACHE_RAM_ATTR2 __section(".ram_code")
#endif
#define ICACHE_RAM_ATTR //nothing//
#else
#undef ICACHE_RAM_ATTR //fix to allow both esp32 and esp8266 to use ICACHE_RAM_ATTR for mapping to IRAM
#define ICACHE_RAM_ATTR IRAM_ATTR
#endif

#if defined(TARGET_NATIVE)
#define IRAM_ATTR
#include "native.h"
#endif

/*
 * Features
 * define features based on pins before defining pins as UNDEF_PIN
 */

// Using these DEBUG_* imply that no SerialIO will be used so the output is readable
#if defined(TARGET_RX) && (defined(DEBUG_RCVR_LINKSTATS) || defined(DEBUG_RX_SCOREBOARD) || defined(DEBUG_RCVR_SIGNAL_STATS))
#define DEBUG_CRSF_NO_OUTPUT
#endif

#if defined(DEBUG_CRSF_NO_OUTPUT)
#define OPT_CRSF_RCVR_NO_SERIAL true
#elif defined(TARGET_RX)
extern bool pwmSerialDefined;

#define OPT_CRSF_RCVR_NO_SERIAL (GPIO_PIN_RCSIGNAL_RX == UNDEF_PIN && GPIO_PIN_RCSIGNAL_TX == UNDEF_PIN && !pwmSerialDefined)
#else
#define OPT_CRSF_RCVR_NO_SERIAL false
#endif

#if defined(USE_ANALOG_VBAT) && !defined(GPIO_ANALOG_VBAT)
#define GPIO_ANALOG_VBAT        A0
#if !defined(ANALOG_VBAT_SCALE)
#define ANALOG_VBAT_SCALE       1
#endif
#endif

#if defined(RADIO_SX128X)
#define Regulatory_Domain_ISM_2400 1
// ISM 2400 band is in use => undefine other requlatory domain defines
#undef Regulatory_Domain_AU_915
#undef Regulatory_Domain_EU_868
#undef Regulatory_Domain_IN_866
#undef Regulatory_Domain_FCC_915
#undef Regulatory_Domain_AU_433
#undef Regulatory_Domain_EU_433
#undef Regulatory_Domain_US_433
#undef Regulatory_Domain_US_433_WIDE

#elif defined(RADIO_SX127X) || defined(RADIO_LR1121)
#if !(defined(Regulatory_Domain_AU_915) || defined(Regulatory_Domain_FCC_915) || \
        defined(Regulatory_Domain_EU_868) || defined(Regulatory_Domain_IN_866) || \
        defined(Regulatory_Domain_AU_433) || defined(Regulatory_Domain_EU_433) || \
        defined(Regulatory_Domain_US_433) || defined(Regulatory_Domain_US_433_WIDE) || \
        defined(UNIT_TEST))
#error "Regulatory_Domain is not defined for 900MHz device. Check user_defines.txt!"
#endif
#else
#error "Either RADIO_SX127X, RADIO_LR1121 or RADIO_SX128X must be defined!"
#endif

#if defined(PLATFORM_ESP32)
#include <soc/uart_pins.h>
#endif
#if !defined(U0RXD_GPIO_NUM)
#define U0RXD_GPIO_NUM (3)
#endif
#if !defined(U0TXD_GPIO_NUM)
#define U0TXD_GPIO_NUM (1)
#endif

#include "hardware.h"
