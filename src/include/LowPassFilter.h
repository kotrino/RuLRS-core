#pragma once
#include "stdint.h"

/////////// Super Simple Fixed Point Lowpass ////////////////

class LPF
{
public:
    int32_t SmoothDataINT;
    int32_t SmoothDataFP;
    int32_t Beta = 3;     // Длина = 16
    int32_t FP_Shift = 5; // Количество дробных битов
    bool NeedReset = true;  // ждем первых данных.

    LPF(int Beta_, int FP_Shift_)
    {
        Beta = Beta_;
        FP_Shift = FP_Shift_;
    }

    LPF(int Beta_)
    {
        Beta = Beta_;
    }

    LPF()
    {
        Beta = 3;
        FP_Shift = 5;
    }

    int32_t ICACHE_RAM_ATTR update(int32_t Indata)
    {
        if (NeedReset)
        {
            init(Indata);
            return SmoothDataINT;
        }

        int RawData;
        RawData = Indata;
        RawData <<= FP_Shift; // Перемещение в фиксированную точку
        SmoothDataFP = (SmoothDataFP << Beta) - SmoothDataFP;
        SmoothDataFP += RawData;
        SmoothDataFP >>= Beta;
        // Не выполняйте следующий сдвиг, если вы хотите делать дальнейшие
        // расчеты в фиксированной точке, используя SmoothData
        SmoothDataINT = SmoothDataFP >> FP_Shift;
        return SmoothDataINT;
    }

    void ICACHE_RAM_ATTR reset()
    {
        NeedReset = true;
    }


    void ICACHE_RAM_ATTR init(int32_t Indata)
    {
        NeedReset = false;

        SmoothDataINT = Indata;
        SmoothDataFP = SmoothDataINT << FP_Shift;
    }

    int32_t value() const { return SmoothDataINT; }
};
