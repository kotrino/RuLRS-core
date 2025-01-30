#include "CRSF.h"

#include "common.h"
#include "FIFO.h"

rulrsLinkStatistics_t CRSF::LinkStatistics;
GENERIC_CRC8 crsf_crc(CRSF_CRC_POLY);

uint8_t CRSF::MspData[RULRS_MSP_BUFFER] = {0};
uint8_t CRSF::MspDataLength = 0;

static const auto MSP_SERIAL_OUT_FIFO_SIZE = 256U;
static FIFO<MSP_SERIAL_OUT_FIFO_SIZE> MspWriteFIFO;

device_type_e CRSF::deviceType = DEVICE_TYPE_RULRS; // По умолчанию RULRS

/***
 * @brief: Преобразует строку версии в целочисленное представление
 * например "2.2.15 ISM24G" => 0x0002020f
 * Предполагается, что все поля версии < 256, числовая часть
 * ДОЛЖНА заканчиваться пробелом для правильного разбора
 ***/
uint32_t CRSF::VersionStrToU32(const char *verStr)
{
    uint32_t retVal = 0;
#if !defined(FORCE_NO_DEVICE_VERSION)
    uint8_t accumulator = 0;
    char c;
    bool trailing_data = false;
    while ((c = *verStr))
    {
        ++verStr;
        // Точка указывает на переход к новому полю версии
        if (c == '.')
        {
            retVal = (retVal << 8) | accumulator;
            accumulator = 0;
            trailing_data = false;
        }
        // Если это число, добавляем его
        else if (c >= '0' && c <= '9')
        {
            accumulator = (accumulator * 10) + (c - '0');
            trailing_data = true;
        }
        // Любой символ кроме [0-9.] завершает разбор
        else
        {
            break;
        }
    }
    if (trailing_data)
    {
        retVal = (retVal << 8) | accumulator;
    }
    // Если ID версии < 1.0.0, мы не смогли разобрать его,
    // используем OTA версию как основной номер версии
    if (retVal < 0x010000)
    {
        retVal = OTA_VERSION_ID << 16;
    }
#endif
    return retVal;
}

void CRSF::GetDeviceInformation(uint8_t *frame, uint8_t fieldCount)
{
    const uint8_t size = strlen(device_name)+1;
    auto *device = (deviceInformationPacket_t *)(frame + sizeof(crsf_ext_header_t) + size);
    // Пакет начинается с имени устройства
    memcpy(frame + sizeof(crsf_ext_header_t), device_name, size);
    // Затем следует информация об устройстве
    device->serialNo = htobe32(deviceType == DEVICE_TYPE_RULRS ? SERIAL_RULRS : SERIAL_ELRS);
    device->hardwareVer = 0; // не используется в данный момент [ 0x00, 0x0b, 0x10, 0x01 ] // "Hardware: V 1.01" / "Bootloader: V 3.06"
    device->softwareVer = htobe32(VersionStrToU32(version)); // версия прошивки [ 0x00, 0x00, 0x05, 0x0f ] // "Firmware: V 5.15"
    device->fieldCnt = fieldCount;
    device->parameterVersion = 0;
}

void CRSF::SetMspV2Request(uint8_t *frame, uint16_t function, uint8_t *payload, uint8_t payloadLength)
{
    auto *packet = (uint8_t *)(frame + sizeof(crsf_ext_header_t));
    packet[0] = 0x50;          // no error, version 2, beginning of the frame, first frame (0)
    packet[1] = 0;             // flags
    packet[2] = function & 0xFF;
    packet[3] = (function >> 8) & 0xFF;
    packet[4] = payloadLength & 0xFF;
    packet[5] = (payloadLength >> 8) & 0xFF;
    memcpy(packet + 6, payload, payloadLength);
    packet[6 + payloadLength] = CalcCRCMsp(packet + 1, payloadLength + 5); // crc = flags + function + length + payload
}

void CRSF::SetHeaderAndCrc(uint8_t *frame, crsf_frame_type_e frameType, uint8_t frameSize, crsf_addr_e destAddr)
{
    auto *header = (crsf_header_t *)frame;
    header->device_addr = destAddr;      // адрес устройства назначения
    header->frame_size = frameSize;      // размер фрейма
    header->type = frameType;            // тип фрейма

    // Вычисляем CRC начиная с байтов после заголовка
    uint8_t crc = crsf_crc.calc(&frame[CRSF_FRAME_NOT_COUNTED_BYTES], frameSize - 1, 0);
    frame[frameSize + CRSF_FRAME_NOT_COUNTED_BYTES - 1] = crc;
}

void CRSF::SetExtendedHeaderAndCrc(uint8_t *frame, crsf_frame_type_e frameType, uint8_t frameSize, crsf_addr_e senderAddr, crsf_addr_e destAddr)
{
    auto *header = (crsf_ext_header_t *)frame;
    header->dest_addr = destAddr;        // адрес получателя
    header->orig_addr = senderAddr;      // адрес отправителя
    SetHeaderAndCrc(frame, frameType, frameSize, destAddr);
}

void CRSF::GetMspMessage(uint8_t **data, uint8_t *len)
{
    *len = MspDataLength;
    *data = (MspDataLength > 0) ? MspData : nullptr;
}

void CRSF::ResetMspQueue()
{
    MspWriteFIFO.flush();
    MspDataLength = 0;
    memset(MspData, 0, RULRS_MSP_BUFFER);
}

void CRSF::UnlockMspMessage()
{
    // текущее MSP сообщение отправлено, восстанавливаем следующую буферизованную запись
    if (MspWriteFIFO.size() > 0)
    {
        MspWriteFIFO.lock();
        MspDataLength = MspWriteFIFO.pop();
        MspWriteFIFO.popBytes(MspData, MspDataLength);
        MspWriteFIFO.unlock();
    }
    else
    {
        // нет MSP сообщений готовых к отправке
        MspDataLength = 0;
        memset(MspData, 0, RULRS_MSP_BUFFER);
    }
}

void ICACHE_RAM_ATTR CRSF::AddMspMessage(mspPacket_t* packet, uint8_t destination)
{
    if (packet->payloadSize > ENCAPSULATED_MSP_MAX_PAYLOAD_SIZE)
    {
        return;
    }

    const uint8_t totalBufferLen = packet->payloadSize + ENCAPSULATED_MSP_HEADER_CRC_LEN + CRSF_FRAME_LENGTH_EXT_TYPE_CRC + CRSF_FRAME_NOT_COUNTED_BYTES;
    uint8_t outBuffer[ENCAPSULATED_MSP_MAX_FRAME_LEN + CRSF_FRAME_LENGTH_EXT_TYPE_CRC + CRSF_FRAME_NOT_COUNTED_BYTES];

    // Расширенный заголовок CRSF фрейма
    outBuffer[0] = CRSF_ADDRESS_BROADCAST;                                      // широковещательный адрес
    outBuffer[1] = packet->payloadSize + ENCAPSULATED_MSP_HEADER_CRC_LEN + CRSF_FRAME_LENGTH_EXT_TYPE_CRC; // длина
    outBuffer[2] = CRSF_FRAMETYPE_MSP_WRITE;                                    // тип пакета
    outBuffer[3] = destination;                                                 // адрес назначения
    outBuffer[4] = CRSF_ADDRESS_RADIO_TRANSMITTER;                              // адрес источника

    // Инкапсулированные MSP данные
    outBuffer[5] = 0x30;                // заголовок
    outBuffer[6] = packet->payloadSize; // размер MSP данных
    outBuffer[7] = packet->function;    // команда пакета
    for (uint16_t i = 0; i < packet->payloadSize; ++i)
    {
        // копируем данные пакета в выходной буфер
        outBuffer[8 + i] = packet->payload[i];
    }
    // Encapsulated MSP crc
    outBuffer[totalBufferLen - 2] = CalcCRCMsp(&outBuffer[6], packet->payloadSize + 2);

    // CRSF frame crc
    outBuffer[totalBufferLen - 1] = crsf_crc.calc(&outBuffer[2], packet->payloadSize + ENCAPSULATED_MSP_HEADER_CRC_LEN + CRSF_FRAME_LENGTH_EXT_TYPE_CRC - 1);
    AddMspMessage(totalBufferLen, outBuffer);
}

void ICACHE_RAM_ATTR CRSF::AddMspMessage(const uint8_t length, uint8_t* data)
{
    if (length > RULRS_MSP_BUFFER)
    {
        return;
    }

    // сохраняем следующее MSP сообщение
    if (MspDataLength == 0)
    {
        for (uint8_t i = 0; i < length; i++)
        {
            MspData[i] = data[i];
        }
        MspDataLength = length;
    }
    // сохраняем все запросы на запись, так как обновление отправляет множество записей
    else
    {
        MspWriteFIFO.lock();
        if (MspWriteFIFO.ensure(length + 1))
        {
            MspWriteFIFO.push(length);
            MspWriteFIFO.pushBytes((const uint8_t *)data, length);
        }
        MspWriteFIFO.unlock();
    }
}

#if defined(CRSF_RX_MODULE)

bool CRSF::HasUpdatedUplinkPower = false;

/***
 * @brief: Вызывается когда новая мощность передачи доступна от TX через OTA
 */
void CRSF::updateUplinkPower(uint8_t uplinkPower)
{
    if (uplinkPower != LinkStatistics.uplink_TX_Power)
    {
        LinkStatistics.uplink_TX_Power = uplinkPower;
        HasUpdatedUplinkPower = true;
    }
}

/***
 * @brief: Возвращает true если HasUpdatedUplinkPower и очищает флаг
 */
bool CRSF::clearUpdatedUplinkPower()
{
    bool retVal = HasUpdatedUplinkPower;
    HasUpdatedUplinkPower = false;
    return retVal;
}

#endif // CRSF_RX_MODULE
