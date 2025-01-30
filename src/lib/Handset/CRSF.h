#ifndef H_CRSF
#define H_CRSF

#include "crsf_protocol.h"
#include "telemetry_protocol.h"
#include "msp.h"
#include "msptypes.h"

// Определения для идентификации устройств
#define SERIAL_ELRS     0x454C5253  // 'ELRS' - серийный номер для ELRS
#define SERIAL_RULRS    0x524C5253  // 'RLRS' - серийный номер для RuLRS

// Перечисление типов устройств
enum device_type_e {
    DEVICE_TYPE_ELRS = 0,  // Тип устройства ELRS
    DEVICE_TYPE_RULRS = 1  // Тип устройства RuLRS
};

class CRSF
{
public:
    static device_type_e deviceType;  // Текущий тип устройства
    static rulrsLinkStatistics_t LinkStatistics; // Статистика соединения хранится как структура

    // Методы для работы с MSP сообщениями
    static void GetMspMessage(uint8_t **data, uint8_t *len);
    static void UnlockMspMessage();
    static void AddMspMessage(uint8_t length, uint8_t *data);
    static void AddMspMessage(mspPacket_t *packet, uint8_t destination);
    static void ResetMspQueue();

    // Методы для работы с информацией об устройстве
    static void GetDeviceInformation(uint8_t *frame, uint8_t fieldCount);
    static void SetMspV2Request(uint8_t *frame, uint16_t function, uint8_t *payload, uint8_t payloadLength);
    static void SetHeaderAndCrc(uint8_t *frame, crsf_frame_type_e frameType, uint8_t frameSize, crsf_addr_e destAddr);
    static void SetExtendedHeaderAndCrc(uint8_t *frame, crsf_frame_type_e frameType, uint8_t frameSize, crsf_addr_e senderAddr, crsf_addr_e destAddr);
    static uint32_t VersionStrToU32(const char *verStr);

#if defined(CRSF_RX_MODULE)
public:
    static void updateUplinkPower(uint8_t uplinkPower);
    static bool clearUpdatedUplinkPower();

private:
    static bool HasUpdatedUplinkPower;
#endif

private:
    static uint8_t MspData[RULRS_MSP_BUFFER];  // Буфер для MSP данных
    static uint8_t MspDataLength;              // Длина MSP данных

public:
    static uint16_t ChannelData[16]; // 16 каналов для управления джойстиком
};

extern GENERIC_CRC8 crsf_crc;

#endif