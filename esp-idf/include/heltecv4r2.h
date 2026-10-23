/**
 * heltecv4r2.h — Heltec WiFi LoRa 32 V4 (R2) board support for reticulous.
 *
 * The V4 R2 is an ESP32-S3R2 (2 MB *quad* PSRAM, 16 MB flash) carrying one
 * Semtech SX1262 on its own SPI bus, a Vext-gated peripheral power rail and a
 * 0.96" SSD1306 OLED. The board wires LoRa, Vext and the OLED (the OLED via
 * spangap/tinylcd — pins published in straddle.yaml, panel powered off Vext);
 * GNSS and SD stay unwired. See heltecv4r2.cpp for the implementation and the
 * board reference: https://heltec.org/project/wifi-lora-32-v4/
 *
 * What this module provides:
 *   - Compile-time constants for the board's own pins (the Vext power-enable).
 *   - The board bring-up service, Heltecv4R2Board.
 *
 * The SX1262's pins (NSS/SCK/MOSI/MISO/RST/BUSY/DIO1, TCXO, DIO2 RF switch)
 * are NOT wired here: they belong to iface-lora's CONFIG_LORA* knobs, set as
 * board VALUES in this straddle's straddle.yaml `kconfig:` block. So this
 * header carries only the board's own power rail.
 */
#pragma once

#include "sdkconfig.h"
#include "service.h"

#define BOARD_NAME              "Heltec WiFi LoRa 32 V4 (R2)"

/* Vext peripheral power-enable. The V4 R2 gates its external 3.3 V rail (Vext
 * pin, OLED, GNSS header) behind a P-MOSFET controlled by GPIO 36: drive the
 * gate LOW to turn the rail ON (active-low). The SX1262 itself is powered
 * directly, not off Vext, so LoRa works regardless — but we still bring the
 * rail up at boot so any Vext-powered peripheral added later is live. -1 = no
 * such pin. */
#define BOARD_VEXT_CTRL_PIN     36
#define BOARD_VEXT_ON_LEVEL     0   /* 0 = pull low to enable (active-low) */

/**
 * Board bring-up, as a registered Service. Heltecv4R2Board::onStart is the
 * always-on hardware bring-up: it drives the Vext peripheral power rail on and
 * parks the LoRa radio's CS line HIGH so the SX1262 doesn't drive MISO before
 * the LoRa interface claims it. It runs in the start band, before spangapInit()
 * — and before tinylcd's task touches the Vext-powered OLED. There is no
 * onInit companion: the OLED UI is tinylcd's own service.
 */
class Heltecv4R2Board : public Service {
public:
    void onStart() override;
};
