#pragma once

#include <stdint.h>
#include <config.h>
#include <POWERMGNT.h>
#include <CRSF.h>
#include <logging.h>

#if defined(TARGET_TX)

#include <MeanAccumulator.h>

#define DYNPOWER_UPDATE_NOUPDATE -128
#define DYNPOWER_UPDATE_MISSED   -127

// Вызов DynamicPower_Init в setup()
void DynamicPower_Init();
// Вызов DynamicPower_Update из loop()
void DynamicPower_Update(uint32_t now);
// Вызов DynamicPower_TelemetryUpdate из ISR с DYNPOWER_UPDATE_MISSED или ScaledSNR значением
void DynamicPower_TelemetryUpdate(int8_t snrScaled);

#endif // TARGET_TX

#if defined(TARGET_RX)

// Вызов DynamicPower_UpdateRx из loop()
void DynamicPower_UpdateRx(bool initialize);

#endif // TARGET_RX
