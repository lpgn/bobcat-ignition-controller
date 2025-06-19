# Flashing the Firmware without the T-U2T Dongle

This document describes how to flash the firmware on the LILYGO TTGO T-Relay board without using the official T-U2T dongle. This method is based on a comment by a user named "Em" on the Amazon product page.

## Required Hardware

* A "USB 3.1 Type-C Male to Female Test Board with 24 Pins Header Connector". This is used to break out the pins from the USB-C connector on the board.
* An ESP8266 code burning fixture with a CP2102 (USB to UART) chip. This is used to communicate with the ESP32 on the board.

## Board Specifics

The LILYGO TTGO T-Relay is a development board featuring:

* **Microcontroller:** ESP32-Wrover-B (with 4MB Flash and 8MB PSRAM)
* **Connectivity:** WiFi 802.11 b/g/n and Bluetooth BLE v4.2
* **Relays:** 4 relays with optocoupler isolation.
* **Interfaces:** UART, SPI, I2C, CAN, I2S, SDIO

The key thing to note is that this board **does not have an onboard USB-to-serial converter**. This is why a separate programmer is required.

## Process

1. **Connect the Hardware:**
    * Connect the USB-C breakout board to the T-Relay board.
    * Connect the following pins from the breakout board to your CP2102 programmer:
        * TX
        * RX
        * VCC
        * GND
        * EN (also known as Chip-PU)
        * IO0 (also known as Boot)

2. **Enter Programming Mode:**
    * Putting the board into programming mode requires a specific timing sequence. You may need to experiment with grounding and ungrounding the EN pin while uploading the code.

3. **Flash the Firmware:**
    * The recommended firmware is Tasmota. You can download the `tasmota32.factory.bin` file from the Tasmota website.
    * Use a tool like `esptool.py` or the Arduino IDE to flash the firmware to the board.

## Direct Soldering / Advanced Flashing

For those comfortable with soldering, you can directly access the necessary programming pins on the board. This is essentially what the breakout board method achieves, but in a more permanent fashion.

You will need to identify the pads for TX, RX, GND, 3.3V, EN, and IO0 on the board. You can then solder wires to these pads and connect them to your USB-to-serial converter.

**It is highly recommended to consult the official board schematic for the exact pin locations.** You can find the schematic in the official LILYGO T-Relay GitHub repository.

## Disclaimer

This method is not officially supported by LILYGO and may require some technical expertise. Proceed with caution, as incorrect wiring or flashing procedures could potentially damage the board.
