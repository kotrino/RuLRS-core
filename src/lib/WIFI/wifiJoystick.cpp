#include "device.h"

#include "wifiJoystick.h"

#if defined(TARGET_TX) && defined(PLATFORM_ESP32)

#include <WiFi.h>
#include <WiFiUdp.h>
#include "common.h"
#include "CRSF.h"
#include "handset.h"
#include "POWERMGNT.h"
#include "hwTimer.h"
#include "logging.h"
#include "options.h"


#if defined(RADIO_SX127X)
extern SX127xDriver Radio;
#elif defined(RADIO_SX128X)
extern SX1280Driver Radio;
#endif

WiFiUDP *WifiJoystick::udp = NULL;
IPAddress WifiJoystick::remoteIP;
uint8_t WifiJoystick::channelCount = JOYSTICK_DEFAULT_CHANNEL_COUNT;
bool WifiJoystick::active = false;
uint8_t WifiJoystick::failedCount = 0;

const uint8_t DEVICE_IDENTIFIER[] = {'R', 'L', 'R', 'S'};

static inline uint16_t htole16(uint16_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return __builtin_bswap16(val);
#else
    return val;
#endif
}

void WifiJoystick::StartJoystickService()
{
    if (!udp)
    {
        udp = new WiFiUDP();
        udp->begin(JOYSTICK_PORT);
    }
}

void WifiJoystick::StopJoystickService()
{
    active = false;
    if (udp)
    {
        udp->stop();
        delete udp;
        udp = NULL;
    }
}

void WifiJoystick::StartSending(const IPAddress& ip, int32_t updateInterval, uint8_t newChannelCount)
{
    remoteIP = ip;
    if (!udp || active)
    {
        return;
    }

    // RF должен быть уже выключен, если в режиме wifi
    // Настроить таймер на работу с запрошенным интервалом
    // с жестким нижним пределом 1000Гц
    if (updateInterval == 0 || updateInterval < 1000)
    {
        updateInterval = JOYSTICK_DEFAULT_UPDATE_INTERVAL;
    }
    hwTimer::updateInterval(updateInterval);
    handset->setPacketInterval(updateInterval);

    handset->setRCDataCallback(UpdateValues);

    // Channel Count
    if (newChannelCount == 0)
    {
        newChannelCount = JOYSTICK_DEFAULT_CHANNEL_COUNT;
    }
    else if (newChannelCount > 16)
    {
        newChannelCount = 16;
    }
    channelCount = newChannelCount;

    active = true;
    failedCount = 0;
}


void WifiJoystick::Loop(unsigned long now)
{
    static unsigned long last = 0;
    if (!udp || active)
    {
       return;
    }

    // Оповещение о существовании сервера через широковещательную рассылку
    constexpr unsigned BROADCAST_INTERVAL = 5000U;
    if (now - last > BROADCAST_INTERVAL)
    {
        last = now;

        struct RulrsUdpAdvertisement_s {
            uint32_t magic;     // магическое число char[4] = ['R', 'L', 'R', 'S']
            uint8_t version;    // версия JOYSTICK_VERSION
            uint16_t port;      // порт JOYSTICK_PORT, little endian
            uint8_t name_len;   // длина имени устройства, которое следует далее
            char name[0];       // имя устройства
        } PACKED eua = {
            .magic = htobe32(0x524C5253), // 'RLRS' в big-endian порядке
            .version = JOYSTICK_VERSION,
            .port = htole16(JOYSTICK_PORT),
            .name_len = (uint8_t)strlen(device_name)
        };

       udp->beginPacket(IPAddress(255, 255, 255, 255), JOYSTICK_PORT);
       udp->write((uint8_t*)&eua, sizeof(eua));
       udp->write((uint8_t*)device_name, eua.name_len);
       udp->endPacket();
    }
}

void WifiJoystick::UpdateValues()
{
    if (!udp || !active)
    {
        return;
    }

    udp->beginPacket(remoteIP, JOYSTICK_PORT);
    udp->write(WifiJoystick::FRAME_CHANNELS);
    udp->write(channelCount);
    for (uint8_t i = 0; i < channelCount; i++)
    {
        uint16_t channel = htole16(map(
            constrain(ChannelData[i], CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_MAX),
            CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_MAX, 0, 0x7fff));
        udp->write((uint8_t*)&channel, 2);
    }

    // проверить, не произошла ли ошибка отправки, не прекращать отправку после первой ошибки,
    // так как могут возникать временные ошибки
    if (udp->endPacket() == 0)
    {
        failedCount++;
        active = failedCount < JOYSTICK_MAX_SEND_ERROR_COUNT;
    }
    else {
        failedCount = 0;
    }
}

#endif
