/***
 * Этот файл определяет интерфейс от модулей устройства к функциям в rx_main или tx_main
 * Используйте его вместо прямого объявления внешних переменных (externs) в вашем модуле
 ***/

#include "common.h"

/***
 * TX интерфейс
 ***/
#if defined(TARGET_TX)
#endif

/***
 * RX интерфейс
 ***/
#if defined(TARGET_RX)
uint8_t getLq();
#endif
