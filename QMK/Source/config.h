/* Copyright 2022 Jose Pablo Ramirez <jp.ramangulo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

/* OLED SPI Defines */
#define OLED_DISPLAY_128X64
#define OLED_IC OLED_IC_SH1106

/* OLED SPI Pins */
#define OLED_DC_PIN GP24
#define OLED_CS_PIN GP22
#define OLED_RST_PIN GP23

/* Shift OLED columns by 2 pixels */
#define OLED_COLUMN_OFFSET 2

/* Divisor for OLED */
#define OLED_SPI_DIVISOR 4

/* ChibiOS SPI definitions */
#define SPI_DRIVER SPID1
#define SPI_SCK_PIN GP26
#define SPI_MOSI_PIN GP27
#define SPI_MISO_PIN GP28

/* Double tap the side button to enter bootloader */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP13
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U

/* Audio */
#define AUDIO_PIN GP16
#define AUDIO_PWM_DRIVER PWMD0
#define AUDIO_PWM_CHANNEL RP2040_PWM_CHANNEL_A
#define AUDIO_INIT_DELAY
#define AUDIO_CLICKY

// USB Device descriptor parameters
//#define VENDOR_ID       0x239A
//#define PRODUCT_ID      0x0108
//#define DEVICE_VER      0x0001
//#define MANUFACTURER    "Adafruit"
//#define PRODUCT         "macropad"
#define DESCRIPTION     "12 key macropad with rotary encoder and OLED screen"
#define RAW_USAGE_PAGE 	0xFF60 // Raw HID communication
#define RAW_USAGE_ID 	0x61 // Raw HID communication

///#define TAPPING_TERM 250 // Speed for double-tap

///define ENCODER_RESOLUTION 4 // Original default was 2, however a value of 4 works better

// RGB LEDs are ws2812
#define RGBLIGHT_LIMIT_VAL 128 // Set max value (brightness) for LEDs (max limit is 255 or RGBLIGHT_LIMIT_VAL)
#define RGBLIGHT_DEFAULT_VAL 0 // Set default value (brightness) for LEDs
#define RGBLIGHT_SLEEP // RGB lighting will be switched off when the host goes to sleep
#define RGB_MATRIX_LED_COUNT 12