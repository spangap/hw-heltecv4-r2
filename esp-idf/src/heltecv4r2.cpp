/**
 * heltecv4r2.cpp — Heltec WiFi LoRa 32 V4 (R2) board support, end to end.
 *
 * Single owner of all V4 R2 hardware bring-up. See heltecv4r2.h for the API contract
 * and the board reference: https://heltec.org/project/wifi-lora-32-v4/. Layout:
 *
 *   1. Vext peripheral power rail + LoRa CS park.
 *      Always compiled. Driven from Heltecv4R2Board::onStart() before spangapInit().
 *
 * The SX1262 lives on its own SPI bus (separate from the flash) and is powered
 * directly, not off Vext — so unlike the T-Deck there is no shared-bus SD probe
 * to race. We still park the radio's CS HIGH before loraInit() owns it, and
 * bring Vext up so any rail-powered peripheral added later is live at boot.
 */
#include "heltecv4r2.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* =========================================================================
 * 1. Vext peripheral power rail + LoRa CS park
 * ========================================================================= */

static void heltecv4r2PowerInit(void)
{
#if BOARD_VEXT_CTRL_PIN >= 0
    gpio_config_t vext = {};
    vext.pin_bit_mask = 1ULL << BOARD_VEXT_CTRL_PIN;
    vext.mode         = GPIO_MODE_OUTPUT;
    vext.pull_up_en   = GPIO_PULLUP_DISABLE;
    vext.pull_down_en = GPIO_PULLDOWN_DISABLE;
    vext.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&vext);
    gpio_set_level((gpio_num_t)BOARD_VEXT_CTRL_PIN, BOARD_VEXT_ON_LEVEL);
    /* Exempt the gate from light-sleep isolation: CONFIG_PM_SLP_DISABLE_GPIO
     * switches every pin to its sleep config on sleep entry, which floats
     * this active-low gate high and cuts the rail mid-sleep — the OLED loses
     * VCC on the first light-sleep entry (e.g. the moment `usb down` releases
     * the console's no-sleep lock) and comes back uninitialised. */
    gpio_sleep_sel_dis((gpio_num_t)BOARD_VEXT_CTRL_PIN);
    vTaskDelay(pdMS_TO_TICKS(100));   /* rail settle */
#endif
    /* Park the SX1262's CS HIGH (deselected) so it doesn't drive MISO before
     * loraInit() claims the pin. The LoRa radio CS pin comes from iface-lora's
     * Kconfig (CONFIG_LORA0_CS_PIN); defined only when iface-lora is staged. */
#if defined(CONFIG_LORA0_CS_PIN)
    gpio_config_t cs = {};
    cs.pin_bit_mask = 1ULL << CONFIG_LORA0_CS_PIN;
    cs.mode         = GPIO_MODE_OUTPUT;
    cs.pull_up_en   = GPIO_PULLUP_DISABLE;
    cs.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cs.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&cs);
    gpio_set_level((gpio_num_t)CONFIG_LORA0_CS_PIN, 1);
#endif
}

/* =========================================================================
 * Public API — the always-on board bring-up (see heltecv4r2.h).
 * ========================================================================= */

void Heltecv4R2Board::onStart() {
    heltecv4r2PowerInit();   /* Vext rail + LoRa CS park */
}
