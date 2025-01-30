#pragma once

#if defined(TARGET_TX) && defined(PLATFORM_ESP32)

#include <WiFiUdp.h>

#define JOYSTICK_PORT 11000
#define JOYSTICK_DEFAULT_UPDATE_INTERVAL 10000
#define JOYSTICK_DEFAULT_CHANNEL_COUNT 8
#define JOYSTICK_VERSION 1
#define JOYSTICK_MAX_SEND_ERROR_COUNT 100

/**
 * Класс для отправки данных стиков через UDP
 * Версия 1
 *
 * Использование для симулятора или драйвера на ПК:
 *
 * Шаг 1: Найти устройство. Есть два варианта:
 *   Рекомендуемый: Использовать MDNS для поиска UDP сервиса rulrs чтобы узнать IP+Port, версия протокола в txt записи сервиса
 *   Альтернативный: Слушать UDP броадкасты, содержащие фрейм со структурой:
 *     4 байта: ['R', 'L', 'R', 'S']
 *     1 байт: Версия протокола
 *     2 байта unsigned: PORT, little-endian
 *     1 байт: длина имени устройства
 *     ASCII текст: Имя устройства
 *
 * Шаг 2:
 *   Начать сокет udp recvfrom(IP, PORT) обнаруженный в Шаге 1
 *
 * Шаг 3:
 *   Отправить HTTP POST запрос к устройству URL http://<IP>/udpcontrol
 *   Param: "action" must be "joystick_begin"
 *   Param (optional): "interval" in us to send updates, or 0 for default (10ms)
 *   Param (optional): "channels" number of channels to send in each frame, or 0 for default (8)
 *   e.g. http://<IP>/udpcontrol?action=joystick_begin&interval=10000&channels=8
 *
 * Шаг 4:
 *   Получить кадры в формате:
 *   1 байт: Тип кадра (WifiJoystickFrameType_e)CHANNELS
 *   1 байт: Количество каналов, следующих за ним
 *   2 байта unsigned * количество каналов: Данные канала в диапазоне от 0 до 0x7fff, little-endian
 *
 * Шаг 5:
 *  Чтобы завершить отправку данных джойстика, отправьте POST на URL управления
 *  Param: "action" must be "joystick_end"
 *  e.g http://<IP>/udpcontrol?action=joystick_end
 */

class WifiJoystick
{
public:
    enum WifiJoystickFrameType_e {
        FRAME_CHANNELS = 1
    };
    static void StartJoystickService();
    static void StopJoystickService();
    static void UpdateValues();
    static void StartSending(const IPAddress& ip, int32_t updateInterval, uint8_t newChannelCount);
    static void StopSending() { active = false; }
    static void Loop(unsigned long now);
private:
    static WiFiUDP *udp;
    static IPAddress remoteIP;
    static uint8_t channelCount;
    static bool active;
    static uint8_t failedCount;
};

// Только объявление, без определения
extern const uint8_t DEVICE_IDENTIFIER[];

#endif
