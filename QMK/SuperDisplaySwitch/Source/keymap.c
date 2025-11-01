// TODO
// convert the port numbers on the OLED into useful names (laptop, pi, PC)
// maybe with each switch element keypress all the (known) selected keys illuminate?
// maybe look into showing small images on the oled instead of pure text when switching elements?
// get some audio code working just to try it out and have it available (though likely comment it out)
// maybe see if it's possible to show the QMK logo only on initialization and then have the oled clear?




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
 
/*
//  __  __                      _____          _   _____  _____ ___   ___  _  _    ___  
// |  \/  |                    |  __ \        | | |  __ \|  __ \__ \ / _ \| || |  / _ \ 
// | \  / | __ _  ___ _ __ ___ | |__) |_ _  __| | | |__) | |__) | ) | | | | || |_| | | |
// | |\/| |/ _` |/ __| '__/ _ \|  ___/ _` |/ _` | |  _  /|  ___/ / /| | | |__   _| | | |
// | |  | | (_| | (__| | | (_) | |  | (_| | (_| | | | \ \| |    / /_| |_| |  | | | |_| |
// |_|  |_|\__,_|\___|_|  \___/|_|   \__,_|\__,_| |_|  \_\_|   |____|\___/   |_|  \___/ 
//																						
// 																	SUPER DISPLAY SWITCH
*/
 
 // *************
// *  SUMMARY  *
// *************

// Custom keymap for MacroPad RP2040 by Exergist (2025)
// Functionality includes:
//   • STUFF
//   • STUFF
//   • STUFF
//   • STUFF
//   • STUFF
 
// **********************
// *  ACKNOWLEDGEMENTS  *
// **********************

// MacroPad RP2040 hardware and original source code by Adafruit (https://learn.adafruit.com/adafruit-macropad-rp2040)
// ASCII art by patorjk (https://patorjk.com/software/taag/, "Big" font)

// **********************
// *  INCLUDE & DEFINE  *
// **********************

#include QMK_KEYBOARD_H
#include "deferred_exec.h"

// ********************************
// *  CUSTOM KEYCODE DECLARATION  *
// ********************************

// Name and assigned unique numbers for custom keycodes
enum customKeycodes
{
	PORT_1 = SAFE_RANGE,
	PORT_2,
	PORT_3,
	KVM_1,
	KVM_2,
	KVM_3,
	USB_1,
	USB_2,
	USB_3,
	AUDIO_1,
	AUDIO_2,
	AUDIO_3,
	PORT_CHECK
};

// ************************
// *  STRUCTURES & ENUMS  *
// ************************

typedef struct { int Port; HSV Color; } KvmpPort;
typedef struct { KvmpPort Device; KvmpPort KVM; KvmpPort Usb; KvmpPort Audio; } KvmpConfig;

typedef enum { DEVICE, KVM, USBHUB, AUDIO, ALL } PortReportingType;

// **********************
// *  GLOBAL VARIABLES  *
// **********************

int illuminationTime = 2000; // Time in milliseconds to illuminate LEDs and OLED screen
static deferred_token led_off_tok[RGBLIGHT_LED_COUNT]; // One deferred token per LED
static deferred_token oled_off_tok; // One deferred token for the OLED screen

KvmpConfig kvmpConfig = { // Initialize KVMP configuration variable	
	.Device = { .Port = -1, .Color = (HSV){HSV_WHITE} },
	.KVM 	= { .Port = -1, .Color = (HSV){HSV_PURPLE} },
    .Usb 	= { .Port = -1, .Color = (HSV){HSV_BLUE} },
    .Audio	= { .Port = -1, .Color = (HSV){HSV_GREEN} }
};


// ************
// *  KEYMAP  *
// ************

// |---------------|
// | Keypad Layout |
// |---------------|

/* 
//				 	 			 	      /\			  		  
//				 	 			 	      ||					  
//				 	 				     USB-C					  
//				 	 				      ||					  
//				 	 				      \/				  	  
//                  .--------------------------------------------.
//                  | .------------------------.        ___      |
//                  | |                        |     .-'   `-.   |
//                  | |                        |    /         \  |
//                  | |       OLED Screen      |   |  Encoder  | |
//                  | |                        |    \         /  |
//   Reset Button  =| |                        |     `-.___.-'   |
//                  | '------------------------'                 |
//                  | .------------..------------..------------. |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | |  Button1   ||  Button2   ||  Button3   | |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | '------------''------------''------------' |
//                  | .------------..------------..------------. |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | |  Button4   ||  Button5   ||  Button6   | |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | '------------''------------''------------' |
//                  | .------------..------------..------------. |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | |  Button7   ||  Button8   ||  Button9   | |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | '------------''------------''------------' |
//                  | .------------..------------..------------. |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | |  Button10  ||  Button11  ||  Button12  | |
//                  | |            ||            ||            | |
//                  | |            ||            ||            | |
//                  | '------------''------------''------------' |
//                  '--------------------------------------------'
*/

// |---------------|
// | Device Keymap |
// |---------------|

// Defines the behavior for encoder and key presses across all applicable layers
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT(
						   PORT_CHECK,
      PORT_1,   PORT_2,    PORT_3,
      KVM_1,    KVM_2,     KVM_3,
      USB_1,    USB_2,     USB_3,
      AUDIO_1,  AUDIO_2,   AUDIO_3
  )
};

// |------------------|
// | Encoder Rotation |
// |------------------|

// Defines the behavior for encoder rotation across all applicable layers
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) } // Format = Counter-clockwise action, clockwise action
};
#endif

// ***********************
// *  METHOD DEFINITION  *
// ***********************

// Method run as the very last task in the keyboard initialization process
void keyboard_post_init_user(void) {
	rgblight_enable_noeeprom(); // Enable RGB LEDs
	rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT); // Set LED mode to solid color
	
	rgblight_sethsv_noeeprom(HSV_GREEN); // Turn on all LEDs
	wait_ms(250); // Brief pause

	// Turn off all LEDs and clear any deferral tokens
    for (uint8_t i = 0; i < RGBLIGHT_LED_COUNT; i++) {
        rgblight_sethsv_at(0, 0, 0, i);
        led_off_tok[i] = INVALID_DEFERRED_TOKEN;
    }
	oled_off_tok = INVALID_DEFERRED_TOKEN;
	
	oled_set_cursor(0, 2); // Position the OLED cursor (21 columns × 8 rows on 128×64 OLED screen)
}

// Turn off one LED when the timer fires
static uint32_t led_off_cb(uint32_t trigger_time, void *cb_arg) {
    uint8_t idx = (uint8_t)(uintptr_t)cb_arg;
    rgblight_sethsv_at(0, 0, 0, idx);
    led_off_tok[idx] = INVALID_DEFERRED_TOKEN;
    return 0; // Don't reschedule
}

// Clear the OLED screen when timer fires
static uint32_t oled_off_cb(uint32_t trigger_time, void *cb_arg) {
	oled_clear();
	oled_off_tok = INVALID_DEFERRED_TOKEN;
	return 0;
}

// Turn on LED now, auto-off after specified time (non-blocking)
static void light_led_for(uint8_t idx, HSV color, uint16_t ms) {
	if (idx != -1) { // Check if inputted light identifier is valid
		uint8_t lightID = idx-1; // Calculate number of target key (or light)
		rgblight_sethsv_at(color.h, color.s, color.v, lightID); // Light up key number lightID with specified color
		if (led_off_tok[lightID] != INVALID_DEFERRED_TOKEN) { // Check if a valid deferral token already exists
			extend_deferred_exec(led_off_tok[lightID], ms); // Extend the existing pending execution relative to "now"
		} else {
			led_off_tok[lightID] = defer_exec(ms, led_off_cb, (void *)(uintptr_t)lightID); // Schedule a fresh deferral
		}
	}
}

// Method for turning off all LEDs while keeping them enabled
static void all_leds_off_noeeprom(void) {
    for (uint8_t i = 0; i < RGBLIGHT_LED_COUNT; i++) { 
        rgblight_sethsv_at(0, 0, 0, i); // Loop through and turn off each LED
    }
}

// Method for flashing RGB LEDs to alert of an issue or unexpected situation
static void error_flash(void)
{	
	float timeStep = 125; // Time step for cycling flashing colors (ms)
	int flashTime = illuminationTime; // Amount of time to flash LEDs (ms)
	float timer = 0;
	
	// Loop over flashTime while cycling displayed colors every timeStep
	while (timer < flashTime)
	{
		rgblight_sethsv_noeeprom(HSV_YELLOW); // Set all LEDs to yellow (without saving to EEPROM)
		wait_ms(timeStep); // Brief pause
		all_leds_off_noeeprom(); // Turn off all LEDs
		wait_ms(timeStep); // Brief pause
		timer += (timeStep * 2); // Increment the flash timer
	}
}

static void port_config_report(PortReportingType type, uint16_t ms)
{
	char line1[16]; // Create a buffer
	oled_clear(); // Clear the OLED screen of content
	
	switch (type) {
		case DEVICE:
			if (kvmpConfig.Device.Port != -1) { // Check if the device port has been changed
				snprintf(line1, sizeof(line1), "Port:%d", kvmpConfig.Device.Port); // Insert content into line1
			} else {
				snprintf(line1, sizeof(line1), "Port:?"); // Insert content into line1
			}
			oled_write_ln(line1, false);  // Write line1 and include newline
			break;
		
		case KVM:
			if (kvmpConfig.KVM.Port != -1) { // Check if the KVM port has been changed
				snprintf(line1, sizeof(line1), "KVM:%d", kvmpConfig.KVM.Port); // Insert content into line1
			} else {
				snprintf(line1, sizeof(line1), "KVM:?"); // Insert content into line1
			}
			oled_write_ln(line1, false);  // Write line1 and include newline
			break;
		
		case USBHUB:
			if (kvmpConfig.Usb.Port != -1) { // Check if the USB hub port has been changed
				snprintf(line1, sizeof(line1), "USB:%d", kvmpConfig.Usb.Port); // Insert content into line1
			} else {
				snprintf(line1, sizeof(line1), "USB:?"); // Insert content into line1
			}
			oled_write_ln(line1, false);  // Write line1 and include newline
			break;
		
		case AUDIO:
			if (kvmpConfig.Audio.Port != -1) { // Check if the audio port has been changed
				snprintf(line1, sizeof(line1), "Audio:%d", kvmpConfig.Audio.Port); // Insert content into line1
			} else {
				snprintf(line1, sizeof(line1), "Audio:?"); // Insert content into line1
			}
			oled_write_ln(line1, false);  // Write line1 and include newline
			break;
		
		case ALL:
			char line2[40], dvcStr[8], kvmStr[8], usbStr[8], audStr[8]; // Create buffers
			
			// Preprocess to format each value for possible insertion of "?"
			snprintf(dvcStr, sizeof(dvcStr), "%s",  kvmpConfig.Device.Port   == -1 ? "?" : "");
			snprintf(kvmStr, sizeof(kvmStr), "%s",  kvmpConfig.KVM.Port   == -1 ? "?" : "");
			snprintf(usbStr, sizeof(usbStr), "%s",  kvmpConfig.Usb.Port   == -1 ? "?" : "");
			snprintf(audStr, sizeof(audStr), "%s",  kvmpConfig.Audio.Port == -1 ? "?" : "");
			
			// Replace empty strings with actual port numbers
			if (dvcStr[0] == '\0') snprintf(dvcStr, sizeof(dvcStr), "%d", kvmpConfig.Device.Port);
			if (kvmStr[0] == '\0') snprintf(kvmStr, sizeof(kvmStr), "%d", kvmpConfig.KVM.Port);
			if (usbStr[0] == '\0') snprintf(usbStr, sizeof(usbStr), "%d", kvmpConfig.Usb.Port);
			if (audStr[0] == '\0') snprintf(audStr, sizeof(audStr), "%d", kvmpConfig.Audio.Port);

			snprintf(line1, sizeof(line1), "Port:%s", dvcStr); // Insert content into line1
			snprintf(line2, sizeof(line2), "KVM:%s USB:%s Audio:%s", kvmStr, usbStr, audStr); // Insert content into line2
			oled_write_ln(line1, false);  // Write line1 to the OLED screen and include newline
			oled_write_ln(line2, false);  // Write line2 to the OLED screen and include newline
			break;
	}
	if (oled_off_tok != INVALID_DEFERRED_TOKEN) { // Check if a valid deferral token already exists
		extend_deferred_exec(oled_off_tok, ms); // Extend the existing pending execution relative to "now"
	} else {
		oled_off_tok = defer_exec(ms, oled_off_cb, NULL); // Schedule a fresh deferral
	}
}

// Method to change ports on the ATEN CS1924 KVMP switch
static void change_port(int portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.Device.Port = kvmpConfig.KVM.Port = kvmpConfig.Usb.Port = kvmpConfig.Audio.Port = ledNumber; // Update kvmpConfig to reflect the port change
	port_config_report(ALL, illuminationTime); // Report port or focus configuration on OLED screen (with auto-clear)
	
	// Turn on relevant keypad LEDs to reflect the port change
	light_led_for(kvmpConfig.Device.Port, kvmpConfig.Device.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected port for a period of time (they will auto-off)
	light_led_for(kvmpConfig.KVM.Port+3, kvmpConfig.KVM.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected KVM-focused port for a period of time (they will auto-off)
	light_led_for(kvmpConfig.Usb.Port+6, kvmpConfig.Usb.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected USB-focused port for a period of time (they will auto-off)
	light_led_for(kvmpConfig.Audio.Port+9, kvmpConfig.Audio.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected USB-focused port for a period of time (they will auto-off)

	// Send series of key taps (macro) to focus on target port on the ATEN CS1924 KVMP switch
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(portHotkey); // Send the keycode corresponding to the target port number
	wait_ms(80);
	tap_code(KC_ENTER);
}

// Method to change only the KVM (keyboard, video, mouse) focus on the ATEN CS1924 KVMP switch to target port
static void change_KVM(uint16_t portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.KVM.Port = ledNumber-3; // Update kvmpConfig to reflect the KVM focus change
	if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all elements are focusing on the same port
		kvmpConfig.Device.Port = kvmpConfig.KVM.Port; // Update kvmpConfig.Device.Port to align with the focus of all other elements
	}
	else {
		kvmpConfig.Device.Port = -1; // Update kvmpConfig.Device.Port to indicate that its value is not set
	}
	port_config_report(KVM, illuminationTime); // Report port or focus configuration on OLED screen (with auto-clear)
	light_led_for(ledNumber, kvmpConfig.KVM.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected KVM-focused port for a period of time (they will auto-off)
	
	// Send series of key taps (macro) to direct the ATEN CS1924 KVMP switch's KVM to focus on a target port
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(portHotkey); // Send the keycode corresponding to the target port number
	wait_ms(80);
	tap_code(KC_K); // Send modifier keycode to trigger a switch in the KVM only
	wait_ms(80);
	tap_code(KC_ENTER);
}

// Method to change only the usb hub focus on the ATEN CS1924 KVMP switch to target port
static void change_usb(uint16_t portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.Usb.Port = ledNumber-6; // Update kvmpConfig to reflect the USB focus change
	if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all elements are focusing on the same port
		kvmpConfig.Device.Port = kvmpConfig.KVM.Port; // Update kvmpConfig.Device.Port to align with the focus of all other elements
	}
	else {
		kvmpConfig.Device.Port = -1; // Update kvmpConfig.Device.Port to indicate that its value is not set
	}
	port_config_report(USBHUB, illuminationTime); // Report port or focus configuration on OLED screen (with auto-clear)
	light_led_for(ledNumber, kvmpConfig.Usb.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected USB-focused port for a period of time (they will auto-off)	
	
	// Send series of key taps (macro) to direct the ATEN CS1924 KVMP switch's USB hub to focus on a target port
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(portHotkey); // Send the keycode corresponding to the target port number
	wait_ms(80);
	tap_code(KC_U); // Send modifier keycode to trigger a switch in the USB hub only
	wait_ms(80);
	tap_code(KC_ENTER);
}

// Method to change only the audio focus on the ATEN CS1924 KVMP switch to target port
static void change_audio(uint16_t portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.Audio.Port = ledNumber-9; // Update kvmpConfig to reflect the Audio focus change
	if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all elements are focusing on the same port
		kvmpConfig.Device.Port = kvmpConfig.KVM.Port; // Update kvmpConfig.Device.Port to align with the focus of all other elements
	}
	else {
		kvmpConfig.Device.Port = -1; // Update kvmpConfig.Device.Port to indicate that its value is not set
	}
	port_config_report(AUDIO, illuminationTime); // Report port or focus configuration on OLED screen (with auto-clear)
	light_led_for(ledNumber, kvmpConfig.Audio.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected audio-focused port for a period of time (they will auto-off)
	
	// Send series of key taps (macro) to direct the ATEN CS1924 KVMP switch's audio to focus on a target port
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(KC_SCROLL_LOCK);
	wait_ms(80);
	tap_code(portHotkey); // Send the keycode corresponding to the target port number
	wait_ms(80);
	tap_code(KC_S); // Send modifier keycode to trigger a switch in the audio only
	wait_ms(80);
	tap_code(KC_ENTER);
}

// Method run whenever a key is pressed or released
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	uint16_t portHotkey = 0;
	uint16_t ledNumber = 0;
    switch (keycode) {
	case PORT_CHECK:
        if (record->event.pressed) {
			// When keycode PORT_CHECK is pressed
			int inputCheck = kvmpConfig.Device.Port + kvmpConfig.KVM.Port + kvmpConfig.Usb.Port + kvmpConfig.Audio.Port;
			if (inputCheck == -4) {
				// MacroPad hasn't been used to send port or focus selection input to ATEN CS1924 KVMP switch
				error_flash(); // Flash error pattern on bigKNOB
			}
			else {
				port_config_report(ALL, illuminationTime); // Report port or focus configuration on OLED screen (with auto-clear)
				
				// Light the keys to display the current KVMP focus
				if (kvmpConfig.Device.Port > 0) {
					light_led_for(kvmpConfig.Device.Port, kvmpConfig.Device.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected port for a period of time (they will auto-off)
				}
				if (kvmpConfig.KVM.Port > 0) {
					light_led_for(kvmpConfig.KVM.Port+3, kvmpConfig.KVM.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected KVM-focused port for a period of time (they will auto-off)
				}
				if (kvmpConfig.Usb.Port > 0) {
					light_led_for(kvmpConfig.Usb.Port+6, kvmpConfig.Usb.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected USB-focused port for a period of time (they will auto-off)
				}
				if (kvmpConfig.Audio.Port > 0) {
					light_led_for(kvmpConfig.Audio.Port+9, kvmpConfig.Audio.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected USB-focused port for a period of time (they will auto-off)
				}
			}
        } else {
            // When keycode PORT_CHECK is released
        }
        return true;
		
    case PORT_1:
        if (record->event.pressed) {
            // When keycode MACRO_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 1; // Store LED number corresponding to pressed key
			change_port(portHotkey, ledNumber); // Call method to change ports on the ATEN CS1924 KVMP switch
        } else {
            // When keycode MACRO_1 is released
        }
        break;

    case PORT_2:
        if (record->event.pressed) {
            // When keycode MACRO_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 2; // Store LED number corresponding to pressed key
			change_port(portHotkey, ledNumber); // Call method to change ports on the ATEN CS1924 KVMP switch
        } else {
            // When keycode MACRO_2 is released
        }
        break;
		
    case PORT_3:
        if (record->event.pressed) {
            // When keycode MACRO_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 3; // Store LED number corresponding to pressed key
			change_port(portHotkey, ledNumber); // Call method to change ports on the ATEN CS1924 KVMP switch
        } else {
            // When keycode MACRO_3 is released
        }
        break;
		
    case KVM_1:
        if (record->event.pressed) {
            // When keycode KVM_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 4; // Store LED number corresponding to pressed key
			change_KVM(portHotkey, ledNumber); // Call method to change KVM focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode KVM_1 is released
        }
        break;
		
	case KVM_2:
        if (record->event.pressed) {
            // When keycode KVM_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 5; // Store LED number corresponding to pressed key
			change_KVM(portHotkey, ledNumber); // Call method to change KVM focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode KVM_2 is released
        }
        break;
		
	case KVM_3:
        if (record->event.pressed) {
            // When keycode KVM_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 6; // Store LED number corresponding to pressed key
			change_KVM(portHotkey, ledNumber); // Call method to change KVM focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode KVM_3 is released
        }
        break;
		
	case USB_1:
        if (record->event.pressed) {
            // When keycode USB_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 7; // Store LED number corresponding to pressed key
			change_usb(portHotkey, ledNumber); // Call method to change USB hub focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode USB_1 is released
        }
        break;
		
	case USB_2:
        if (record->event.pressed) {
            // When keycode USB_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 8; // Store LED number corresponding to pressed key
			change_usb(portHotkey, ledNumber); // Call method to change USB hub focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode USB_2 is released
        }
        break;
		
	case USB_3:
        if (record->event.pressed) {
            // When keycode USB_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 9; // Store LED number corresponding to pressed key
			change_usb(portHotkey, ledNumber); // Call method to change USB hub focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode USB_3 is released
        }
        break;
		
	case AUDIO_1:
        if (record->event.pressed) {
            // When keycode AUDIO_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 10; // Store LED number corresponding to pressed key
			change_audio(portHotkey, ledNumber); // Call method to change audio focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode AUDIO_1 is released
        }
        break;
		
	case AUDIO_2:
        if (record->event.pressed) {
            // When keycode AUDIO_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 11; // Store LED number corresponding to pressed key
			change_audio(portHotkey, ledNumber); // Call method to change audio focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode AUDIO_2 is released
        }
        break;

	case AUDIO_3:
        if (record->event.pressed) {
            // When keycode AUDIO_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 12; // Store LED number corresponding to pressed key
			change_audio(portHotkey, ledNumber); // Call method to change audio focus on the ATEN CS1924 KVMP switch
        } else {
            // When keycode AUDIO_3 is released
        }
        break;
    }
    return true;
}

// **********
// *  OLED  *
// **********

/* #ifdef OLED_ENABLE

static void render_qmk_logo(void) {
	static const char PROGMEM qmk_logo[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x3f, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
		0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x83, 0x83, 0x83, 0x83, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe,
		0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfe, 0xfe, 0xfe, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x83, 0x83, 0x83, 0x83, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x81, 0x83, 0x83, 0x83, 0x83, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
		0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x83, 0x83, 0x83, 0x83, 0x81,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x07, 0x1f, 0x3f, 0x7f, 0x7e, 0xf8, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xff, 0xff,
		0xff, 0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xf8, 0x7e, 0x7f, 0x3f, 0x1f, 0x07, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xc0, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0xff,
		0xff, 0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc0,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xfc, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfc,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

  oled_write_raw_P(qmk_logo, sizeof(qmk_logo));
}

bool oled_task_user(void) {
	render_qmk_logo();
	return true;
}

#endif */



// ---------------------------------------------------------------------------------------------------------
// ARCHIVE
// ---------------------------------------------------------------------------------------------------------

/* // Method for writing integer debug content to the OLED screen
static void oled_debug_int(int input)
{
	oled_clear();
	oled_set_cursor(2, 1);
	///char buf[8];  // plenty for -32768..32767
	char line[24];
	///snprintf(buf, sizeof(buf), "%d", kvmpConfig.KVM.Port);
	snprintf(line, sizeof(line), "KVM:%d USB:%d Aud:%d", kvmpConfig.KVM.Port, kvmpConfig.Usb.Port, kvmpConfig.Audio.Port);
	oled_write(line, false);  // write without newline
} */

/* // Method for writing string debug content to the OLED screen
static void oled_debug_string(const char *input)
{
	oled_clear(); // Clear the OLED screen of content
	oled_set_cursor(2, 1); // Position the OLED cursor (21 columns × 8 rows on 128×64 OLED screen)
	oled_write(input, false); // Write input to the OLED screen at the cursor location
} */