// Serial
#define GPIO_PIN_RCSIGNAL_RX hardware_pin(HARDWARE_serial_rx)
#define GPIO_PIN_RCSIGNAL_TX hardware_pin(HARDWARE_serial_tx)

// Radio
#define GPIO_PIN_BUSY hardware_pin(HARDWARE_radio_busy)
#define GPIO_PIN_BUSY_2 hardware_pin(HARDWARE_radio_busy_2)
#define GPIO_PIN_DIO0 hardware_pin(HARDWARE_radio_dio0)
#define GPIO_PIN_DIO0_2 hardware_pin(HARDWARE_radio_dio0_2)
#define GPIO_PIN_DIO1 hardware_pin(HARDWARE_radio_dio1)
#define GPIO_PIN_DIO1_2 hardware_pin(HARDWARE_radio_dio1_2)
#define GPIO_PIN_MISO hardware_pin(HARDWARE_radio_miso)
#define GPIO_PIN_MOSI hardware_pin(HARDWARE_radio_mosi)
#define GPIO_PIN_NSS hardware_pin(HARDWARE_radio_nss)
#define GPIO_PIN_NSS_2 hardware_pin(HARDWARE_radio_nss_2)
#define GPIO_PIN_RST hardware_pin(HARDWARE_radio_rst)
#define GPIO_PIN_RST_2 hardware_pin(HARDWARE_radio_rst_2)
#define GPIO_PIN_SCK hardware_pin(HARDWARE_radio_sck)
#define OPT_USE_HARDWARE_DCDC hardware_flag(HARDWARE_radio_dcdc)
#define OPT_USE_SX1276_RFO_HF hardware_flag(HARDWARE_radio_rfo_hf)

// Radio Antenna
#define GPIO_PIN_ANT_CTRL hardware_pin(HARDWARE_ant_ctrl)
#define GPIO_PIN_ANT_CTRL_COMPL hardware_pin(HARDWARE_ant_ctrl_compl)

// Radio power
#define GPIO_PIN_PA_ENABLE hardware_pin(HARDWARE_power_enable)
#define GPIO_PIN_RFamp_APC2 hardware_pin(HARDWARE_power_apc2)
#define GPIO_PIN_RX_ENABLE hardware_pin(HARDWARE_power_rxen)
#define GPIO_PIN_TX_ENABLE hardware_pin(HARDWARE_power_txen)
#define GPIO_PIN_RX_ENABLE_2 UNDEF_PIN
#define GPIO_PIN_TX_ENABLE_2 UNDEF_PIN
#define LBT_RSSI_THRESHOLD_OFFSET_DB hardware_int(HARDWARE_power_lna_gain)
#define MinPower (PowerLevels_e)hardware_int(HARDWARE_power_min)
#define MaxPower (PowerLevels_e)hardware_int(HARDWARE_power_max)
#define DefaultPower (PowerLevels_e)hardware_int(HARDWARE_power_default)

#define POWER_OUTPUT_DACWRITE false // Не поддерживается на оборудовании на базе 8285
#define POWER_OUTPUT_VALUES hardware_i16_array(HARDWARE_power_values)
#define POWER_OUTPUT_VALUES_COUNT hardware_int(HARDWARE_power_values_count)
#define POWER_OUTPUT_VALUES2 hardware_i16_array(HARDWARE_power_values)
#define POWER_OUTPUT_VALUES_DUAL hardware_i16_array(HARDWARE_power_values_dual)
#define POWER_OUTPUT_VALUES_DUAL_COUNT hardware_int(HARDWARE_power_values_dual_count)

// Input
#define GPIO_PIN_BUTTON hardware_pin(HARDWARE_button)
#define GPIO_PIN_BUTTON2 UNDEF_PIN

// Lighting
#define GPIO_PIN_LED_BLUE hardware_pin(HARDWARE_led_blue)
#define GPIO_LED_BLUE_INVERTED hardware_pin(HARDWARE_led_blue_invert)
#define GPIO_PIN_LED_GREEN hardware_pin(HARDWARE_led_green)
#define GPIO_LED_GREEN_INVERTED hardware_flag(HARDWARE_led_green_invert)
#define GPIO_PIN_LED_RED hardware_pin(HARDWARE_led)
#define GPIO_LED_RED_INVERTED hardware_pin(HARDWARE_led_red_invert)

#define GPIO_PIN_LED_WS2812 hardware_pin(HARDWARE_led_rgb)
#define OPT_WS2812_IS_GRB hardware_flag(HARDWARE_led_rgb_isgrb)
#define WS2812_STATUS_LEDS hardware_i16_array(HARDWARE_ledidx_rgb_status)
#define WS2812_STATUS_LEDS_COUNT hardware_int(HARDWARE_ledidx_rgb_status_count)
#define WS2812_VTX_STATUS_LEDS hardware_i16_array(HARDWARE_ledidx_rgb_vtx)
#define WS2812_VTX_STATUS_LEDS_COUNT hardware_int(HARDWARE_ledidx_rgb_vtx_count)
#define WS2812_BOOT_LEDS hardware_i16_array(HARDWARE_ledidx_rgb_boot)
#define WS2812_BOOT_LEDS_COUNT hardware_int(HARDWARE_ledidx_rgb_boot_count)

// Неподдерживаемые функции TX для 8285 TX
#define OPT_HAS_THERMAL false
#define GPIO_PIN_FAN_EN UNDEF_PIN
#define GPIO_PIN_FAN_PWM UNDEF_PIN

#define OPT_USE_TX_BACKPACK false
#define GPIO_PIN_BACKPACK_EN UNDEF_PIN
#define BACKPACK_LOGGING_BAUD 0
#define GPIO_PIN_DEBUG_RX UNDEF_PIN
#define GPIO_PIN_DEBUG_TX UNDEF_PIN

#define OPT_HAS_GSENSOR false
#define OPT_HAS_GSENSOR_STK8xxx false
#define GPIO_PIN_GSENSOR_INT UNDEF_PIN
