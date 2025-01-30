#pragma once

#ifndef UNIT_TEST
#include "targets.h"
#include <device.h>

#if defined(RADIO_SX127X)
#include "SX127xDriver.h"
#elif defined(RADIO_LR1121)
#include "LR1121Driver.h"
#elif defined(RADIO_SX128X)
#include "SX1280Driver.h"
#else
#error "Radio configuration is not valid!"
#endif

#endif // UNIT_TEST

// Используется для XOR с OtaCrcInitializer и macSeed для уменьшения совместимости с предыдущими версиями.
// Должно быть увеличено при изменении структуры пакета OTA.
#define OTA_VERSION_ID      4
#define UID_LEN             6

typedef enum : uint8_t
{
    TLM_RATIO_STD = 0,   // Use suggested ratio from ModParams
    TLM_RATIO_NO_TLM,
    TLM_RATIO_1_128,
    TLM_RATIO_1_64,
    TLM_RATIO_1_32,
    TLM_RATIO_1_16,
    TLM_RATIO_1_8,
    TLM_RATIO_1_4,
    TLM_RATIO_1_2,
    TLM_RATIO_DISARMED, // TLM_RATIO_STD when disarmed, TLM_RATIO_NO_TLM when armed
} rulrs_tlm_ratio_e;

typedef enum
{
    connected,
    tentative,        // RX only
    awaitingModelId,  // TX only
    disconnected,
    MODE_STATES,
    // States below here are special mode states
    noCrossfire,
    bleJoystick,
    NO_CONFIG_SAVE_STATES,
    wifiUpdate,
    serialUpdate,
    // Failure states go below here to display immediately
    FAILURE_STATES,
    radioFailed,
    hardwareUndefined
} connectionState_e;

/**
 * На TX отслеживает, что делать при срабатывании таймера Tock
 **/
typedef enum
{
    ttrpTransmitting,     // Передача RC каналов в нормальном режиме
    ttrpPreReceiveGap,    // Переключился в режим приема телеметрии, но находится в промежутке между окончанием передачи и Tock
    ttrpExpectingTelem    // Все еще в режиме приема, Tock сработал, получаем телеметрию насколько нам известно
} TxTlmRcvPhase_e;

typedef enum
{
    tim_disconnected = 0,
    tim_tentative = 1,
    tim_locked = 2
} RXtimerState_e;

typedef enum
{
    RF_DOWNLINK_INFO = 0,
    RF_UPLINK_INFO = 1,
    RF_AIRMODE_PARAMETERS = 2
} rulrs_tlm_header_e;

typedef enum : uint8_t
{
    // RATE_MODULATION_BAND_RATE_MODE

    RATE_LORA_900_25HZ = 0,
    RATE_LORA_900_50HZ,
    RATE_LORA_900_100HZ,
    RATE_LORA_900_100HZ_8CH,
    RATE_LORA_900_150HZ,
    RATE_LORA_900_200HZ,
    RATE_LORA_900_200HZ_8CH,
    RATE_LORA_900_250HZ,
    RATE_LORA_900_333HZ_8CH,
    RATE_LORA_900_500HZ,
    RATE_LORA_900_50HZ_DVDA,
    RATE_FSK_900_1000HZ_8CH,

    RATE_LORA_2G4_25HZ = 20,
    RATE_LORA_2G4_50HZ,
    RATE_LORA_2G4_100HZ,
    RATE_LORA_2G4_100HZ_8CH,
    RATE_LORA_2G4_150HZ,
    RATE_LORA_2G4_200HZ,
    RATE_LORA_2G4_200HZ_8CH,
    RATE_LORA_2G4_250HZ,
    RATE_LORA_2G4_333HZ_8CH,
    RATE_LORA_2G4_500HZ,
    RATE_FLRC_2G4_250HZ_DVDA,
    RATE_FLRC_2G4_500HZ_DVDA,
    RATE_FLRC_2G4_500HZ,
    RATE_FLRC_2G4_1000HZ,
    RATE_FSK_2G4_250HZ_DVDA,
    RATE_FSK_2G4_500HZ_DVDA,
    RATE_FSK_2G4_1000HZ,
    
    RATE_LORA_DUAL_100HZ_8CH = 100,
    RATE_LORA_DUAL_150HZ,
} rulrs_RFrates_e;

enum {
    RADIO_TYPE_SX127x_LORA,
    RADIO_TYPE_LR1121_LORA_900,
    RADIO_TYPE_LR1121_LORA_2G4,
    RADIO_TYPE_LR1121_GFSK_900,
    RADIO_TYPE_LR1121_GFSK_2G4,
    RADIO_TYPE_LR1121_LORA_DUAL,
    RADIO_TYPE_SX128x_LORA,
    RADIO_TYPE_SX128x_FLRC,
};

typedef enum : uint8_t
{
    TX_RADIO_MODE_GEMINI = 0,
    TX_RADIO_MODE_ANT_1 = 1,
    TX_RADIO_MODE_ANT_2 = 2,
    TX_RADIO_MODE_SWITCH = 3
} tx_radio_mode_e;

typedef enum : uint8_t
{
    TX_NORMAL_MODE      = 0,
    TX_MAVLINK_MODE     = 1,
} tx_transmission_mode_e;

// Значение используется для rulrs_rf_pref_params_s.DynpowerUpThresholdSnr если SNR не должен использоваться
#define DYNPOWER_SNR_THRESH_NONE -127
#define SNR_SCALE(snr) ((int8_t)((float)snr * RADIO_SNR_SCALE))
#define SNR_DESCALE(snrScaled) (snrScaled / RADIO_SNR_SCALE)
// Привязка есть, если любой из последних 4 байтов не нулевой (непривязанное состояние - все нули)
#define UID_IS_BOUND(uid) (uid[2] != 0 || uid[3] != 0 || uid[4] != 0 || uid[5] != 0)

typedef struct rulrs_rf_pref_params_s
{
    uint8_t index;
    int16_t RXsensitivity;                // ожидаемая минимальная чувствительность RF
    uint16_t TOA;                         // время в эфире в микросекундах
    uint16_t DisconnectTimeoutMs;         // Время без пакета до перехода приемника в состояние отключения (мс)
    uint16_t RxLockTimeoutMs;             // Максимальное время перехода из предварительного в подключенное состояние на приемнике (мс)
    uint16_t SyncPktIntervalDisconnected; // как часто отправлять PACKET_TYPE_SYNC (мс) когда нет ответа от RX
    uint16_t SyncPktIntervalConnected;    // как часто отправлять PACKET_TYPE_SYNC (мс) когда есть соединение
    int8_t DynpowerSnrThreshUp;           // Запрос на увеличение мощности, если отчетный (средний) SNR находится на этом уровне или ниже
                                          // или DYNPOWER_UPTHRESH_SNR_NONE для использования RSSI
    int8_t DynpowerSnrThreshDn;           // Аналогично DynpowerSnrUpThreshold, но для снижения мощности

} rulrs_rf_pref_params_s;

typedef struct rulrs_mod_settings_s
{
    uint8_t index;
    uint8_t radio_type;
    rulrs_RFrates_e enum_rate;
    uint8_t bw;                    // ширина полосы
    uint8_t sf;                    // коэффициент распространения
    uint8_t cr;                    // скорость кодирования
    uint8_t PreambleLen;           // длина преамбулы
#if defined(RADIO_LR1121)
    uint8_t bw2;                   // ширина полосы 2
    uint8_t sf2;                   // коэффициент распространения 2
    uint8_t cr2;                   // скорость кодирования 2
    uint8_t PreambleLen2;          // длина преамбулы 2
#endif
    rulrs_tlm_ratio_e TLMinterval; // каждый X пакет является ответным пакетом TLM, должен быть степенью 2
    uint8_t FHSShopInterval;       // каждые X пакетов мы переходим на новую частоту. Максимальное значение 16, так как в sync-пакете выделено только 4 бита
    int32_t interval;              // интервал в микросекундах, соответствующий этой частоте
    uint8_t PayloadLength;         // Количество OTA байт для отправки
    uint8_t numOfSends;            // Количество пакетов для отправки
} rulrs_mod_settings_t;

// Ограничено 16 возможными действиями из-за текущего хранения конфигурации
typedef enum : uint8_t {
    ACTION_NONE,
    ACTION_INCREASE_POWER,        // Увеличить мощность
    ACTION_GOTO_VTX_BAND,        // Перейти на диапазон VTX
    ACTION_GOTO_VTX_CHANNEL,     // Перейти на канал VTX
    ACTION_SEND_VTX,             // Отправить VTX
    ACTION_START_WIFI,           // Запустить WiFi
    ACTION_BIND,                 // Привязка
    ACTION_BLE_JOYSTICK,         // Джойстик BLE
    ACTION_RESET_REBOOT,         // Сброс и перезагрузка

    ACTION_LAST
} action_e;

enum eServoOutputMode : uint8_t
{
    som50Hz = 0,    // 0:  50 Гц   | режимы "Servo PWM", где сигнал 988-2012мкс
    som60Hz,        // 1:  60 Гц   | и режим устанавливает интервал обновления
    som100Hz,       // 2:  100 Гц  | должен быть режим=0 по умолчанию в конфигурации
    som160Hz,       // 3:  160 Гц
    som333Hz,       // 4:  333 Гц
    som400Hz,       // 5:  400 Гц
    som10KHzDuty,   // 6:  10кГц duty
    somOnOff,       // 7:  Цифровой режим 0/1
    somDShot,       // 8:  DShot300
    somSerial,      // 9:  primary Serial
    somSCL,         // 10: сигнал тактирования I2C
    somSDA,         // 11: линия данных I2C
    somPwm,         // 12: настоящий режим PWM (НЕ ПОДДЕРЖИВАЕТСЯ)
#if defined(PLATFORM_ESP32)
    somSerial1RX,   // 13: вторичный Serial RX
    somSerial1TX,   // 14: вторичный Serial TX
#endif
};

enum eServoOutputFailsafeMode : uint8_t
{
    PWMFAILSAFE_SET_POSITION,  // user customizable pulse value
    PWMFAILSAFE_NO_PULSES,     // stop pulsing
    PWMFAILSAFE_LAST_POSITION, // continue to pulse last used value
};

enum eSerialProtocol : uint8_t
{
    PROTOCOL_CRSF,
    PROTOCOL_INVERTED_CRSF,
    PROTOCOL_SBUS,
    PROTOCOL_INVERTED_SBUS,
	PROTOCOL_SUMD,
    PROTOCOL_DJI_RS_PRO,
    PROTOCOL_HOTT_TLM,
    PROTOCOL_MAVLINK,
    PROTOCOL_MSP_DISPLAYPORT,
    PROTOCOL_GPS
};

#if defined(PLATFORM_ESP32)
enum eSerial1Protocol : uint8_t
{
    PROTOCOL_SERIAL1_OFF,
    PROTOCOL_SERIAL1_CRSF,
    PROTOCOL_SERIAL1_INVERTED_CRSF,
    PROTOCOL_SERIAL1_SBUS,
    PROTOCOL_SERIAL1_INVERTED_SBUS,
	PROTOCOL_SERIAL1_SUMD,
    PROTOCOL_SERIAL1_DJI_RS_PRO,
    PROTOCOL_SERIAL1_HOTT_TLM,
    PROTOCOL_SERIAL1_TRAMP,
    PROTOCOL_SERIAL1_SMARTAUDIO,
    PROTOCOL_SERIAL1_MSP_DISPLAYPORT,
    PROTOCOL_SERIAL1_GPS
};
#endif

enum eFailsafeMode : uint8_t
{
    FAILSAFE_NO_PULSES,
    FAILSAFE_LAST_POSITION,
    FAILSAFE_SET_POSITION
};

enum eAuxChannels : uint8_t
{
    AUX1 = 4,
    AUX2 = 5,
    AUX3 = 6,
    AUX4 = 7,
    AUX5 = 8,
    AUX6 = 9,
    AUX7 = 10,
    AUX8 = 11,
    AUX9 = 12,
    AUX10 = 13,
    AUX11 = 14,
    AUX12 = 15,
    CRSF_NUM_CHANNELS = 16
};

// СПЕЦИФИЧНЫЙ ДЛЯ ELRS OTA CRC
// Форматирование Купмана https://users.ece.cmu.edu/~koopman/crc/
#define RULRS_CRC_POLY 0x07 // 0x83
#define RULRS_CRC14_POLY 0x2E57 // 0x372B

#ifndef UNIT_TEST
#if defined(RADIO_SX127X)
#define RATE_MAX 6                              // Максимальное количество поддерживаемых скоростей
#define RATE_BINDING RATE_LORA_900_50HZ         // Скорость для режима привязки

extern SX127xDriver Radio;

#elif defined(RADIO_LR1121)
#define RATE_MAX 20                             // Максимальное количество поддерживаемых скоростей
#define RATE_BINDING RATE_LORA_900_50HZ         // Скорость для режима привязки
#define RATE_DUALBAND_BINDING RATE_LORA_2G4_50HZ // Скорость для режима привязки в двухдиапазонном режиме

extern LR1121Driver Radio;

#elif defined(RADIO_SX128X)
#define RATE_MAX 10                             // 2xFLRC + 2xDVDA + 4xLoRa + 2xFullRes
#define RATE_BINDING RATE_LORA_2G4_50HZ         // Скорость для режима привязки

extern SX1280Driver Radio;
#endif
#endif // UNIT_TEST

// Функции для работы с параметрами радио
rulrs_mod_settings_s *get_rulrs_airRateConfig(uint8_t index);        // Получить конфигурацию скорости передачи
rulrs_rf_pref_params_s *get_rulrs_RFperfParams(uint8_t index);      // Получить параметры производительности RF
uint8_t get_rulrs_HandsetRate_max(uint8_t rateIndex, uint32_t minInterval); // Получить максимальную скорость передатчика

// Функции для работы с телеметрией
uint8_t TLMratioEnumToValue(rulrs_tlm_ratio_e const enumval);       // Преобразование перечисления в значение
uint8_t TLMBurstMaxForRateRatio(uint16_t const rateHz, uint8_t const ratioDiv); // Максимальное количество пакетов телеметрии
uint8_t enumRatetoIndex(rulrs_RFrates_e const eRate);               // Преобразование перечисления скорости в индекс

// Глобальные переменные
extern uint8_t UID[UID_LEN];                    // Уникальный идентификатор
extern bool connectionHasModelMatch;            // Флаг соответствия модели
extern bool teamraceHasModelMatch;              // Флаг соответствия модели для командной гонки
extern bool InBindingMode;                      // Флаг режима привязки
extern uint8_t RuLRS_currTlmDenom;             // Текущий знаменатель телеметрии
extern rulrs_mod_settings_s *RuLRS_currAirRate_Modparams;     // Текущие параметры модуляции
extern rulrs_rf_pref_params_s *RuLRS_currAirRate_RFperfParams; // Текущие параметры производительности RF
extern uint32_t ChannelData[CRSF_NUM_CHANNELS]; // Текущее состояние каналов в формате CRSF

extern connectionState_e connectionState;
#if !defined(UNIT_TEST)
inline void setConnectionState(connectionState_e newState) {
    connectionState = newState;
    devicesTriggerEvent(EVENT_CONNECTION_CHANGED); // Вызов события изменения соединения
}
#endif

// Функции для работы с UID и радио
uint32_t uidMacSeedGet();                      // Получить seed на основе UID
bool isDualRadio();                            // Проверка на двухдиапазонный режим
void EnterBindingModeSafely();                 // Безопасный вход в режим привязки

#if defined(RADIO_LR1121)
bool isSupportedRFRate(uint8_t index);         // Проверка поддержки скорости RF
#else
inline bool isSupportedRFRate(uint8_t index) { return true; } // Все скорости поддерживаются
#endif