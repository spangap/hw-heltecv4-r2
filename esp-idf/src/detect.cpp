/**
 * detect.cpp — is the hardware under this firmware a Heltec WiFi LoRa 32 V4 R2?
 *
 * The board's one self-assertion: its own name when the OLED and the radio both
 * answer on Heltec pins, NULL otherwise. See hw-lilygo-tdeck/esp-idf/src/
 * detect.cpp for the contract both callers hold it to, and detect_probe.h for
 * why flashmon's detector carries a hand-kept copy (as `detect_hw_heltecv4_r2`).
 *
 * The V4 ships as two modules on one PCB layout: the R2 (ESP32-S3R2, 2 MB quad
 * PSRAM) and the R8 (ESP32-S3R8, 8 MB octal PSRAM, hw-heltecv4-r8). The eFuses
 * say which before a pin is touched — and must, because GPIO 36, this board's
 * Vext gate, is an octal-PSRAM data line on the R8.
 *
 * The rails: Vext gates the OLED's power and the OLED needs a reset pulse before
 * it will answer, so both are driven here. They are released ONLY when the probe
 * fails — on the board this actually is, the firmware wants them exactly as this
 * left them, and the detector is reset into real firmware straight after.
 */
#include "detect_probe.h"
#include "heltecv4r2.h"

/* OLED — I2C on its own pins, at whichever of the two strapped addresses. */
#define DETECT_OLED_SDA   17
#define DETECT_OLED_SCL   18
#define DETECT_OLED_RST   21

/* LoRa header (straddle.yaml's CONFIG_LORA0_*, written out: those symbols only
 * exist when iface-lora is staged, and this must probe without it). */
#define DETECT_LORA_SCK    9
#define DETECT_LORA_MOSI  10
#define DETECT_LORA_MISO  11
#define DETECT_LORA_CS     8
#define DETECT_LORA_RST   12
#define DETECT_LORA_BUSY  13

extern "C" const char* detect_hw(void)
{
    if (!detect_flash_mb(16)) return NULL;
    if (!detect_psram_mb(2)) return NULL;

    detect_rail_drive(BOARD_VEXT_CTRL_PIN, 0);     /* Vext on (active low) */
    detect_rail_drive(DETECT_OLED_RST, 0);         /* pulse the OLED reset */
    esp_rom_delay_us(5000);
    gpio_set_level((gpio_num_t)DETECT_OLED_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    /* Anchor: the SSD1306 OLED, which answers at 0x3C or 0x3D depending on one
     * strap. A bare ACK is all it offers. */
    if (!detect_ack2(DETECT_OLED_SDA, DETECT_OLED_SCL, 0x3C, 0x3D)) {
        detect_dbg("no OLED on 17/18 — not a Heltec V4 R2");
        detect_rail_release(DETECT_OLED_RST);
        detect_rail_release(BOARD_VEXT_CTRL_PIN);
        return NULL;
    }
    /* The modem, by name. An OLED on 17/18 with a radio on this header is not
     * enough to settle it: the Meshnology W12 carries the same 16 MB flash, the
     * same panel pins and the same LoRa header, and differs by the part on the
     * end of it — an SX1262 here, an LR2021 there. */
    if (!detect_radio_is(DETECT_LORA_SCK, DETECT_LORA_MOSI, DETECT_LORA_MISO,
                         DETECT_LORA_CS, DETECT_LORA_RST, DETECT_LORA_BUSY, "sx1262")) {
        detect_dbg("OLED answered but no SX1262 — not a Heltec V4 R2");
        detect_rail_release(DETECT_OLED_RST);
        detect_rail_release(BOARD_VEXT_CTRL_PIN);
        return NULL;
    }

    detect_found("hw_heltecv4_r2");
    return "hw-heltecv4-r2";
}
