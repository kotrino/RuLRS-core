#include "devAnalogVbat.h"

#include <Arduino.h>
#include "CRSF.h"
#include "telemetry.h"
#include "median.h"
#include "logging.h"

// Выборка 5 значений за 500мс (если не включен SlowUpdate)
#define VBAT_SMOOTH_CNT         5
#if defined(DEBUG_VBAT_ADC)
#define VBAT_SAMPLE_INTERVAL    20U // более частые обновления в режиме отладки
#else
#define VBAT_SAMPLE_INTERVAL    100U
#endif

typedef uint16_t vbatAnalogStorage_t;
static MedianAvgFilter<vbatAnalogStorage_t, VBAT_SMOOTH_CNT>vbatSmooth;
static uint8_t vbatUpdateScale;

#if defined(PLATFORM_ESP32)
#include "esp_adc_cal.h"
static esp_adc_cal_characteristics_t *vbatAdcUnitCharacterics;
#endif

/* Внешние зависимости */
extern Telemetry telemetry;

/**
 * @brief: Включить режим SlowUpdate для уменьшения частоты отправки телеметрии Vbat
 ***/
void Vbat_enableSlowUpdate(bool enable)
{
    vbatUpdateScale = enable ? 2 : 1;
}

static bool initialize()
{
    return GPIO_ANALOG_VBAT != UNDEF_PIN;
}

static int start()
{
    vbatUpdateScale = 1;
#if defined(PLATFORM_ESP32)
    analogReadResolution(12);

    int atten = hardware_int(HARDWARE_vbat_atten);
    if (atten != -1)
    {
        // если настроенное значение выше максимального элемента (11dB, это указывает на использование cal_characterize)
        bool useCal = atten > ADC_11db;
        if (useCal)
        {
            atten -= (ADC_11db + 1);

            vbatAdcUnitCharacterics = new esp_adc_cal_characteristics_t();
            int8_t channel = digitalPinToAnalogChannel(GPIO_ANALOG_VBAT);
            adc_unit_t unit = (channel > (SOC_ADC_MAX_CHANNEL_NUM - 1)) ? ADC_UNIT_2 : ADC_UNIT_1;
            esp_adc_cal_characterize(unit, (adc_atten_t)atten, ADC_WIDTH_BIT_12, 3300, vbatAdcUnitCharacterics);
        }
        analogSetPinAttenuation(GPIO_ANALOG_VBAT, (adc_attenuation_t)atten);
    }
#endif

    return VBAT_SAMPLE_INTERVAL;
}

static void reportVbat()
{
    uint32_t adc = vbatSmooth.calc();
#if defined(PLATFORM_ESP32) && !defined(DEBUG_VBAT_ADC)
    if (vbatAdcUnitCharacterics)
        adc = esp_adc_cal_raw_to_voltage(adc, vbatAdcUnitCharacterics);
#endif

    int32_t vbat;
    // Для отрицательных смещений, любое значение между abs(OFFSET) и 0 считается как 0
    if (ANALOG_VBAT_OFFSET < 0 && adc <= -ANALOG_VBAT_OFFSET)
        vbat = 0;
    else
        vbat = ((int32_t)adc - ANALOG_VBAT_OFFSET) * 100 / ANALOG_VBAT_SCALE;

    CRSF_MK_FRAME_T(crsf_sensor_battery_t) crsfbatt = { 0 };
    // Значения в формате MSB first (BigEndian)
    crsfbatt.p.voltage = htobe16((uint16_t)vbat);
    // Нет датчиков для тока, емкости или оставшегося заряда

    CRSF::SetHeaderAndCrc((uint8_t *)&crsfbatt, CRSF_FRAMETYPE_BATTERY_SENSOR, CRSF_FRAME_SIZE(sizeof(crsf_sensor_battery_t)), CRSF_ADDRESS_CRSF_TRANSMITTER);
    telemetry.AppendTelemetryPackage((uint8_t *)&crsfbatt);
}

static int timeout()
{
    if (telemetry.GetCrsfBatterySensorDetected())
    {
        return DURATION_NEVER;
    }

    uint32_t adc = analogRead(GPIO_ANALOG_VBAT);
#if defined(PLATFORM_ESP32) && defined(DEBUG_VBAT_ADC)
    // При DEBUG_VBAT_ADC каждое значение корректируется (для логирования)
    // в обычном режиме корректируется только финальное значение для экономии CPU
    if (vbatAdcUnitCharacterics)
        adc = esp_adc_cal_raw_to_voltage(adc, vbatAdcUnitCharacterics);
    DBGLN("$ADC,%u", adc);
#endif

    unsigned int idx = vbatSmooth.add(adc);
    if (idx == 0 && connectionState == connected)
        reportVbat();

    return VBAT_SAMPLE_INTERVAL * vbatUpdateScale;
}

device_t AnalogVbat_device = {
    .initialize = initialize,
    .start = start,
    .event = nullptr,
    .timeout = timeout,
    .subscribe = EVENT_NONE
};
