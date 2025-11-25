/*
//  __  __                      _____          _   _____  _____ ___   ___  _  _    ___  
// |  \/  |                    |  __ \        | | |  __ \|  __ \__ \ / _ \| || |  / _ \ 
// | \  / | __ _  ___ _ __ ___ | |__) |_ _  __| | | |__) | |__) | ) | | | | || |_| | | |
// | |\/| |/ _` |/ __| '__/ _ \|  ___/ _` |/ _` | |  _  /|  ___/ / /| | | |__   _| | | |
// | |  | | (_| | (__| | | (_) | |  | (_| | (_| | | | \ \| |    / /_| |_| |  | | | |_| |
// |_|  |_|\__,_|\___|_|  \___/|_|   \__,_|\__,_| |_|  \_\_|   |____|\___/   |_|  \___/ 
//																						
// 																	ULTRA KVMP SWITCH
*/

/* Copyright (c) 2025 Exergist

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */
 
// *************
// *  SUMMARY  *
// *************

// Custom keymap for MacroPad RP2040 by Exergist (2025)
// Functionality includes:
//   • Unified hotkey control of ATEN CS1924 KVMP switch, CS1824 KVMP switch, and CS62KM KM switch (port select, KVM-only, USB-only, audio-only, configuration check)
//	 	○ CS62KM directs input to CS1924 or CS1824 to control either KVMP
//   • OLED UI with device/port status and icons (fast, pixel-accurate blits; page-aligned)
//   • RGB key lighting as visual feedback for current state
//   • Timed auto-off for OLED and LEDs using deferred callbacks
//   • Rotary encoder mapped to system volume (though this isn't supported by the CS62KM)
//   • Sleep/wake handling to clear/re-init OLED and LEDs

// **********************
// *  ACKNOWLEDGEMENTS  *
// **********************

// MacroPad RP2040 hardware and original source code by Adafruit (https://learn.adafruit.com/adafruit-macropad-rp2040)
// ASCII art by patorjk (https://patorjk.com/software/taag/, "Big" font)
// "Laptop" image created by Dong Gyu Yang from Noun Project (https://thenounproject.com/icon/laptop-8086742/)

// **********************
// *  INCLUDE & DEFINE  *
// **********************

#include QMK_KEYBOARD_H
#include "deferred_exec.h"
#include "process_tap_dance.h" 

// ********************************
// *  CUSTOM KEYCODE DECLARATION  *
// ********************************

// Name and assigned unique numbers for custom keycodes
enum customKeycodes
{
	/* PORT_1 = SAFE_RANGE,
	PORT_2,
	PORT_3, */
	KVM_1 = SAFE_RANGE,
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

typedef struct { int Port; HSV Color; } KvmpSwitch;
typedef struct { KvmpSwitch Device; KvmpSwitch KVM; KvmpSwitch Usb; KvmpSwitch Audio; const char *Name; } KvmpConfig;
typedef struct { KvmpSwitch Device; } KmConfig;
typedef enum { DEVICE, KVM, USBHUB, AUDIO, ALL } PortReportingType;

// **********************
// *  GLOBAL VARIABLES  *
// **********************

int illuminationTime = 2000; // Time in milliseconds to illuminate LEDs and OLED screen
int keyPressDelay = 25; // Time in milliseconds to wait between virtually releasing a pressed (tapped) key
static deferred_token led_off_tok[RGBLIGHT_LED_COUNT]; // One deferred token per LED
static deferred_token oled_off_tok; // One deferred token for the OLED screen
static HSV errorColor = { HSV_YELLOW }; // HSV flash color for showing errors

KvmpConfig kvmp1Config = { // Initialize KVMP 1 (CS1924) configuration variable	
	.Device = { .Port = -1, .Color = (HSV){HSV_WHITE} },
	.KVM 	= { .Port = -1, .Color = (HSV){HSV_PURPLE} },
    .Usb 	= { .Port = -1, .Color = (HSV){HSV_BLUE} },
    .Audio	= { .Port = -1, .Color = (HSV){HSV_GREEN} },
	.Name	= "CS1924"
};

KvmpConfig kvmp2Config = { // Initialize KVMP 2 (CS1824) configuration variable	
	.Device = { .Port = -1, .Color = (HSV){HSV_WHITE} },
	.KVM 	= { .Port = -1, .Color = (HSV){HSV_PURPLE} },
    .Usb 	= { .Port = -1, .Color = (HSV){HSV_BLUE} },
    .Audio	= { .Port = -1, .Color = (HSV){HSV_GREEN} },
	.Name	= "CS1824"
};

KmConfig kmConfig = { // Initialize KM (CS62KM) configuration variable	
	.Device = { .Port = -1, .Color = (HSV){HSV_PINK} }
};

// ************************
// *  METHOD DECLARATION  *
// ************************

// Only declared here as needed
///void error_flash(HSV color);
void kvmp_config_report(KvmpConfig kvmpConfig, PortReportingType type, uint16_t ms);
void change_KM_port(int portNumber);
void change_KVMP_port(KvmpConfig kvmpConfig, int portHotkey, int ledNumber);
void kvmp1_check(void);
void kvmp2_check(void);

// ***************
// *  TAP DANCE  *
// ***************

#ifdef TAP_DANCE_ENABLE

// |------------------------|
// | Tap Dance Declarations |
// |------------------------|

// Tap Dance identifier declaration
enum
{
	ENCODER_DANCE,
	PORT1_DANCE,
    PORT2_DANCE,
    PORT3_DANCE
};

// Tap Dance action type declaration
typedef enum
{
	TD_NONE,
	TD_UNKNOWN,
	TD_SINGLE_TAP,
	TD_SINGLE_HOLD,
	TD_DOUBLE_TAP
} td_state_t;

// Tap Dance state information Struct declaration
typedef struct
{
	bool is_press_action;
	td_state_t state;
} td_tap_t;

// |-----------------------|
// | Tap Dance Definitions |
// |-----------------------|

// Method for retrieving requested Tap Dance state based on user input
td_state_t CurrentDance(tap_dance_state_t *state)
{
    if (state->count == 1) // Single-tap
	{
        if (state->interrupted || !state->pressed) // Check if key press was interrupted OR key state is not "pressed" (being held down)
			return TD_SINGLE_TAP; // Single-tap confirmed
        else // Key has not been interrupted AND the key is being held
			return TD_SINGLE_HOLD; // Means the type of tap is actually a "HOLD"
    }
	else if (state->count == 2) // Double-tap
		return TD_DOUBLE_TAP;
	else
		return TD_UNKNOWN;
}

// Create an instance of td_tap_t for the encoder Tap Dance
static td_tap_t encoderTap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

// Create an instance of td_tap_t for the Port1 Tap Dance
static td_tap_t port1Tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

// Create an instance of td_tap_t for the Port2 Tap Dance
static td_tap_t port2Tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

// Create an instance of td_tap_t for the Port3 Tap Dance
static td_tap_t port3Tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

// Method run when Encoder Tap Dance action finishes
void EncoderTapFinished(tap_dance_state_t *state, void *user_data)
{
    encoderTap_state.state = CurrentDance(state);
    switch (encoderTap_state.state)
	{
		case TD_SINGLE_TAP: {
			kvmp1_check(); // Get configuration of kvmp1 (CS1924)
			break;
		}
		case TD_SINGLE_HOLD:
			kvmp2_check(); // Get configuration of kvmp1 (CS1924)
			break;
		case TD_DOUBLE_TAP:
			// do nothing
			break;
		case TD_NONE:
			// do nothing
			break;
		case TD_UNKNOWN:
			// do nothing
			break;
    }
	encoderTap_state.state = TD_NONE; // Reset encoderTap_state
}

// Method run when Port1 Tap Dance action finishes
void Port1TapFinished(tap_dance_state_t *state, void *user_data)
{
    port1Tap_state.state = CurrentDance(state);
    switch (port1Tap_state.state)
    {
        case TD_SINGLE_TAP:
            // Single tap: KM -> port 1 (CS1924), KVMP -> port 1
            change_KM_port(1);
            change_KVMP_port(kvmp1Config, KC_1, 1);
            break;

        case TD_SINGLE_HOLD:
            // Hold: KM -> port 2 (CS1824), KVMP -> port 1
            change_KM_port(2);
            change_KVMP_port(kvmp2Config, KC_1, 1);
            break;

        default:
            break;
    }
    port1Tap_state.state = TD_NONE;
}

// Method run when Port2 Tap Dance action finishes
void Port2TapFinished(tap_dance_state_t *state, void *user_data)
{
    port2Tap_state.state = CurrentDance(state);
    switch (port2Tap_state.state)
    {
        case TD_SINGLE_TAP:
            // Single tap: KM -> port 1 (CS1924), KVMP -> port 2
            change_KM_port(1);
            change_KVMP_port(kvmp1Config, KC_2, 2);
            break;

        case TD_SINGLE_HOLD:
            // Hold: KM -> port 2 (CS1824), KVMP -> port 2
            change_KM_port(2);
            change_KVMP_port(kvmp2Config, KC_2, 2);
            break;

        default:
            break;
    }
    port2Tap_state.state = TD_NONE;
}

// Method run when Port3 Tap Dance action finishes
void Port3TapFinished(tap_dance_state_t *state, void *user_data)
{
    port3Tap_state.state = CurrentDance(state);
    switch (port3Tap_state.state)
    {
        case TD_SINGLE_TAP:
            // Single tap: KM -> port 1 (CS1924), KVMP -> port 3
            change_KM_port(1);
            change_KVMP_port(kvmp1Config, KC_3, 3);
            break;

        case TD_SINGLE_HOLD:
            // Hold: KM -> port 2 (CS1824), KVMP -> port 3
            change_KM_port(2);
            change_KVMP_port(kvmp2Config, KC_3, 3);
            break;

        default:
            break;
    }
    port3Tap_state.state = TD_NONE;
}

// Tap Dance action definition
tap_dance_action_t  tap_dance_actions[] = {
    [ENCODER_DANCE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, EncoderTapFinished, NULL),
	[PORT1_DANCE]   = ACTION_TAP_DANCE_FN_ADVANCED(NULL, Port1TapFinished, NULL),
    [PORT2_DANCE]   = ACTION_TAP_DANCE_FN_ADVANCED(NULL, Port2TapFinished, NULL),
    [PORT3_DANCE]   = ACTION_TAP_DANCE_FN_ADVANCED(NULL, Port3TapFinished, NULL)
};

#endif

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
											TD(ENCODER_DANCE),
      TD(PORT1_DANCE), 	TD(PORT2_DANCE), 	TD(PORT3_DANCE),
      KVM_1,    	   	KVM_2,     			KVM_3,
      USB_1,    	   	USB_2,     			USB_3,
      AUDIO_1,  	   	AUDIO_2,   			AUDIO_3
  )
};

// |------------------|
// | Encoder Rotation |
// |------------------|

#ifdef ENCODER_MAP_ENABLE

// Defines the behavior for encoder rotation across all applicable layers
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) } // Format = Counter-clockwise action, clockwise action
};

#endif

// ***********************
// *  METHOD DEFINITION  *
// ***********************

// |-------------|
// | OLED Screen |
// |-------------|

#ifdef OLED_ENABLE

// Clear the OLED screen when timer fires
static uint32_t oled_off_cb(uint32_t trigger_time, void *cb_arg) {
	oled_clear(); // Clear the OLED screen
	oled_off_tok = INVALID_DEFERRED_TOKEN; // Reset the OLED screen deferral token
	return 0;
}

/*  Draws a monochrome bitmap onto the OLED at a specific pixel location.
	The image must be page-aligned vertically (i.e., y is a multiple of 8).
	Each byte in 'img' represents one vertical column of 8 pixels.

	Arguments:
	img  = pointer to PROGMEM bitmap data (1bpp vertical columns)
	w    = width of the image in pixels (bytes per row)
	h    = height of the image in pixels
	x,y  = pixel coordinates where the image should start (top-left corner)
	ms   = duration in milliseconds before the OLED auto-off timer resets */
static void oled_blit_P(const uint8_t *img, uint8_t w, uint8_t h, uint8_t x, uint8_t y, uint16_t ms) {

    // Each "page" is 8 vertical pixels tall (since 1 byte = 8 bits = 8 vertical pixels)
    // So the number of pages the image spans = ceil(h / 8)
    const uint8_t pages = (h + 7) >> 3;

    // Compute the first byte index in OLED memory where drawing will begin.
    // Each page on the display is OLED_DISPLAY_WIDTH bytes wide (128 for most displays).
    // (y >> 3) converts pixel Y into a page number, and we offset by 'x' horizontally.
    const uint16_t base = ((uint16_t)(y >> 3) * OLED_DISPLAY_WIDTH) + x;

    // Loop through each 8-pixel-high page of the image
    for (uint8_t p = 0; p < pages; ++p) {

        // Pointer to the start of this page's image data in PROGMEM
        // Each page in the image is 'w' bytes wide
        const uint8_t *s = img + (uint16_t)p * w;

        // Compute where this page begins in the OLED's framebuffer memory
        // Move down one full display page (OLED_DISPLAY_WIDTH bytes) per iteration
        uint16_t d = base + (uint16_t)p * OLED_DISPLAY_WIDTH;

        // Loop across all horizontal bytes in this image page
        for (uint8_t i = 0; i < w; ++i) {
            // Read one byte from program memory (8 vertical pixels)
            uint8_t b = pgm_read_byte(s++);
            // Write it directly to the OLED's RAM at absolute address 'd'
            oled_write_raw_byte(b, d++);
        }
    }

	// OLED auto-off timer management
    if (oled_off_tok != INVALID_DEFERRED_TOKEN) { // Check if an OLED auto-off deferral is already scheduled
        extend_deferred_exec(oled_off_tok, ms); // Extend the existing deferral's timer
    } else {
        oled_off_tok = defer_exec(ms, oled_off_cb, NULL); // Start a new deferred callback to clear the OLED display after specified period of time
    }
}

/* 	40x40 Raspberry Pi silhouette, 1-bit, page-ordered, LSB = top pixel
	Draw using oled_blit_P(imgName, width, height, x, y, illuminationTime) */
static const uint8_t PROGMEM raspi40x40[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc2, 0x02, 0x02, 0x03, 0x01, 0x03, 0x03, 0x43, 0x02, 
	0x82, 0x04, 0x8c, 0xf0, 0xf8, 0x04, 0x06, 0x82, 0x42, 0x03, 0x01, 0x01, 0x03, 0x02, 0x02, 0x82, 
	0xfe, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 
	0xe3, 0x36, 0x1c, 0x08, 0x88, 0xc8, 0xe8, 0xfc, 0x8e, 0x0f, 0x07, 0x07, 0x07, 0x0f, 0x0f, 0x9c, 
	0xe8, 0xc8, 0x88, 0x18, 0x18, 0x34, 0xe2, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xe0, 0x38, 0x0c, 0x07, 0x04, 0xfe, 0x0e, 0x03, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0xc7, 0xff, 0xef, 0x81, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0xfe, 0x04, 0x07, 
	0x0c, 0x38, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x38, 0xfc, 
	0x07, 0x07, 0x07, 0x0e, 0x1c, 0x3c, 0xfc, 0x86, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x86, 
	0xfe, 0x3c, 0x0e, 0x06, 0x07, 0x03, 0x07, 0xcc, 0x7c, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x04, 0x08, 0x18, 0x18, 0x3f, 0x6f, 
	0x47, 0xc6, 0x86, 0x86, 0x86, 0xc6, 0x47, 0x67, 0x3f, 0x18, 0x18, 0x08, 0x0c, 0x04, 0x02, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* 	64x64 Raspberry Pi silhouette, 1-bit, page-ordered, LSB = top pixel
	Draw using oled_blit_P(imgName, width, height, x, y, illuminationTime) */
static const uint8_t PROGMEM raspi64x64[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xec, 0x0c, 0x06, 0x06, 0x06, 
	0x06, 0x07, 0x03, 0x03, 0x07, 0x03, 0x06, 0x02, 0x06, 0x06, 0x0e, 0x0c, 0x0c, 0x38, 0xf0, 0xc0, 
	0xc0, 0xe0, 0x38, 0x18, 0x0c, 0x0e, 0x06, 0x06, 0x02, 0x06, 0x03, 0x03, 0x03, 0x03, 0x07, 0x06, 
	0x06, 0x06, 0x06, 0x0c, 0xac, 0xf8, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x3c, 0x70, 0xc0, 0x80, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x18, 0xf0, 0xf0, 0xf0, 0xf8, 0xff, 
	0xff, 0xfd, 0xf0, 0xf0, 0xf0, 0x18, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0xc0, 0x60, 0x3c, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0x31, 
	0x1b, 0x0e, 0x0e, 0x0e, 0x04, 0x84, 0xc4, 0xe6, 0xfe, 0xbe, 0x0f, 0x07, 0x07, 0x03, 0x03, 0x03, 
	0x03, 0x03, 0x03, 0x07, 0x07, 0x0f, 0xbf, 0xee, 0xc6, 0x84, 0x04, 0x0c, 0x0e, 0x0e, 0x1e, 0x3b, 
	0x71, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x7f, 0x7f, 0xf0, 0xf0, 
	0xf8, 0x78, 0x3c, 0x1e, 0x0f, 0x07, 0x07, 0x07, 0x03, 0x07, 0x07, 0x06, 0x0e, 0x1c, 0x7c, 0xfc, 
	0xfc, 0x7c, 0x0c, 0x06, 0x06, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x06, 0x0c, 0x18, 0x78, 0xf0, 
	0xf0, 0x7f, 0x7f, 0x7f, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x07, 0x01, 0x00, 0x00, 0x00, 0xe0, 0xff, 
	0xff, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf8, 0xff, 
	0xff, 0xfc, 0xf0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xff, 
	0xff, 0x80, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x7c, 0xf8, 0xf8, 0x0e, 0x07, 0x07, 
	0x07, 0x0f, 0x0f, 0x1f, 0x3e, 0x7e, 0xfe, 0xfe, 0xfe, 0x0f, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0xbf, 0xfe, 0xfe, 0x3e, 0x1f, 0x0f, 0x07, 0x07, 0x03, 
	0x03, 0x07, 0x0c, 0xf8, 0xfc, 0x7e, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x3c, 0x70, 0xe0, 
	0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x81, 0xc7, 0xff, 0xfe, 0xf8, 0xf0, 0xf0, 0x60, 0x60, 0x60, 
	0x60, 0x60, 0x60, 0xe0, 0xf0, 0xf8, 0xfc, 0xff, 0x83, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 
	0xe0, 0x70, 0x3c, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x01, 0x03, 0x07, 0x07, 0x0f, 0x0f, 0x1f, 0x39, 0x30, 0x60, 0x60, 0xc0, 0xc0, 0xc0, 
	0xc0, 0xc0, 0xc0, 0x60, 0x60, 0x30, 0x39, 0x1f, 0x0f, 0x0f, 0x07, 0x07, 0x03, 0x01, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* 	40x40 Laptop silhouette, 1-bit, page-ordered, LSB = top pixel
	Draw using oled_blit_P(imgName, width, height, x, y, illuminationTime) */
static const uint8_t PROGMEM laptop40x40[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
	0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
	0x60, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	0xc0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xf0, 0xf8, 0xcd, 0xc5, 0xc5, 
	0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0x45, 0x45, 0x45, 0x45, 0x45, 0x45, 0x45, 0x45, 
	0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xcd, 0xf8, 0xf0, 0xc0, 0x80, 0x00, 
	0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00
};

/*  64x64 Laptop silhouette, 1-bit, page-ordered, LSB = top pixel
	Draw using oled_blit_P(imgName, width, height, x, y, illuminationTime) */
static const uint8_t PROGMEM laptop64x64[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0xfe, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0xfe, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x7f, 0x70, 0x70, 0x70, 0x70, 0x70, 
	0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 
	0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 
	0x70, 0x70, 0x70, 0x70, 0x70, 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xf8, 0xfc, 0x9e, 0x8e, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x8e, 0x9e, 0xfc, 0xf8, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x78, 0x7c, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x77, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 
	0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x77, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7c, 0x78, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* 	40x40 PC silhouette, 1-bit, page-ordered, LSB = top pixel
	Draw using oled_blit_P(imgName, width, height, x, y, illuminationTime) */
static const uint8_t PROGMEM pc40x40[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xfe, 
	0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
	0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0xc0, 0xc0, 0xe0, 0xf8, 0xfc, 0xfc, 
	0xfc, 0xfc, 0xcc, 0xc4, 0x84, 0xc4, 0xfc, 0xfc, 0xfc, 0xfc, 0xf8, 0xe0, 0xc0, 0xc0, 0xc0, 0xff, 
	0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*  64x64 PC silhouette, 1-bit, page-ordered, LSB = top pixel
	Draw using oled_blit_P(imgName, width, height, x, y, illuminationTime) */
static const uint8_t PROGMEM pc64x64[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0x0e, 0x06, 
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
	0x06, 0x0e, 0xfe, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x1c, 0x1c, 
	0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 
	0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 
	0x1c, 0x3c, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 
	0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x60, 0x60, 
	0x60, 0x60, 0x60, 0x78, 0x7c, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x61, 0x60, 0x40, 
	0x40, 0x60, 0x71, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7c, 0x70, 0x60, 0x60, 0x60, 
	0x60, 0x70, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Create an array of pointers to all 40x40 images
static const uint8_t * const image40x40_table[] PROGMEM = {
    laptop40x40,
	raspi40x40,
    pc40x40
};

// Method to display the QMK logo on the OLED screen
static void render_qmk_logo(void) {
	static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };
	
	oled_set_cursor(0, 3); // Position the OLED cursor (21 columns × 8 rows on 128×64 OLED screen)
	oled_write_P(qmk_logo, false); // Write the QMK logo on the OLED screen
	
	/* static const char PROGMEM qmk_logo[] = {
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
	
	oled_write_raw_P(qmk_logo, sizeof(qmk_logo)); // Write the QMK logo on the OLED screen */
}

#endif

// |-------|
// | Audio |
// |-------|

#ifdef AUDIO_ENABLE

///static float startup_song[][2] = SONG(STARTUP_SOUND);
///static float goodbye_song[][2] = SONG(GOODBYE_SOUND);

#endif

// |----------------|
// | Initialization |
// |----------------|

// Turn off LEDs and clear OLED after the boot splash
static uint32_t boot_effects_off_cb(uint32_t trigger_time, void *cb_arg) {
    // Turn LEDs off and clear any deferral tokens
    for (uint8_t i = 0; i < RGBLIGHT_LED_COUNT; i++) {
        rgblight_sethsv_at(0, 0, 0, i);
        led_off_tok[i] = INVALID_DEFERRED_TOKEN;
    }
	
    oled_clear(); // Clear the OLED screen
    oled_off_tok = INVALID_DEFERRED_TOKEN; // Reset the OLED screen deferral token
    return 0;  // Don't reschedule
}

// Method run as the very last task in the keyboard initialization process
void keyboard_post_init_user(void) {
	rgblight_enable_noeeprom(); // Enable RGB LEDs
	rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT); // Set LED mode to solid color
	
	// Show QMK logo on OLED screen
	oled_clear(); // Clear the OLED screen
	render_qmk_logo(); // Show the QMK logo
	
	// Turn on all LEDs
	rgblight_sethsv_noeeprom(HSV_GREEN); 

    // Schedule LEDs and OLED screen to turn off after brief period of time
    oled_off_tok = defer_exec(illuminationTime, boot_effects_off_cb, NULL);

	// Position the OLED cursor (21 columns × 8 rows on 128×64 OLED screen)
	oled_set_cursor(0, 2);
	
	/* // Play a startup audio song
	PLAY_SONG(startup_song); */
}

// |---------------|
// | MacroPad LEDs |
// |---------------|

#ifdef RGBLIGHT_ENABLE

// Turn off one LED when the timer fires
static uint32_t led_off_cb(uint32_t trigger_time, void *cb_arg) {
    uint8_t idx = (uint8_t)(uintptr_t)cb_arg; // Get the ID of the triggering key
    rgblight_sethsv_at(0, 0, 0, idx); // Turn of the LED of the triggering key
    led_off_tok[idx] = INVALID_DEFERRED_TOKEN; // Reset the deferral token for the triggering key's LED
    return 0; // Don't reschedule
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

/* // Method for flashing RGB LEDs to alert of an issue or unexpected situation
void error_flash(HSV color)
{	
	float timeStep = 125; // Time step for cycling flashing colors (ms)
	int flashTime = illuminationTime; // Amount of time to flash LEDs (ms)
	float timer = 0;
	
	// Loop over flashTime while cycling displayed colors every timeStep
	while (timer < flashTime)
	{
		rgblight_sethsv_noeeprom(color.h, color.s, color.v); // Set all LEDs to specified HSV color (without saving to EEPROM)
		wait_ms(timeStep); // Brief pause
		all_leds_off_noeeprom(); // Turn off all LEDs
		wait_ms(timeStep); // Brief pause
		timer += (timeStep * 2); // Increment the flash timer
	}
} */

// Method for flashing RGB LEDs to alert of an issue or unexpected situation (in deferred context)
static uint32_t deferred_error_flash_callback(uint32_t trigger_time, void *cb_arg) {
    HSV color = *(HSV *)cb_arg; // Capture passed-in color for the error_flash
    ///error_flash(color); // Call error_flash with inputted color
	
	float timeStep = 125; // Time step for cycling flashing colors (ms)
	int flashTime = illuminationTime; // Amount of time to flash LEDs (ms)
	float timer = 0;
	
	// Loop over flashTime while cycling displayed colors every timeStep
	while (timer < flashTime)
	{
		rgblight_sethsv_noeeprom(color.h, color.s, color.v); // Set all LEDs to specified HSV color (without saving to EEPROM)
		wait_ms(timeStep); // Brief pause
		all_leds_off_noeeprom(); // Turn off all LEDs
		wait_ms(timeStep); // Brief pause
		timer += (timeStep * 2); // Increment the flash timer
	}
	
    return 0; // Don't reschedule
}

#endif

// |------------------|
// | CS62KM Switching |
// |------------------|

// Method to change ports on the ATEN CS62KM KM switch
void change_KM_port(int portNumber)
{	
	if (kmConfig.Device.Port != portNumber) {
		kmConfig.Device.Port = portNumber; // Update kmConfig to reflect the port change
		uint16_t portHotkey = (portNumber == 1 ? KC_1 : KC_2); // Store keycode corresponding to the target port number
		
		// Send series of key taps (macro) to focus on target port on the ATEN CS62KM KM switch
		tap_code_delay(KC_RIGHT_CTRL, keyPressDelay);
		tap_code_delay(KC_RIGHT_CTRL, keyPressDelay);
		tap_code_delay(portHotkey, keyPressDelay); // Send the keycode corresponding to the target port number
		tap_code_delay(KC_ENTER, keyPressDelay);
	}
}

// |----------------------------------|
// | ATEN CS1924 and CS1824 Switching |
// |----------------------------------|

// Helper method to pre-format port information for output on the OLED screen
static const char *format_port_output(int p, char *buf, size_t n) {
    if (p < 0) return "?"; // Check if the port number p is less than 0 (and if so return "?")
    snprintf(buf, n, "%d", p); // Store the port number p in buf
    return buf; // Return buf
}

// Method to output ATEN CS1924/CS1824 port or focus status via imagery on the OLED screen and key backlights
void kvmp_config_report(KvmpConfig kvmpConfig, PortReportingType type, uint16_t ms)
{
	char line1[64]; // Create a buffer
	oled_clear(); // Clear the OLED screen of content
	
	switch (type) {
		case DEVICE:
			if (kvmpConfig.Device.Port != -1) { // Check if the device port has been changed
				// Write the device images
				if (kvmpConfig.Device.Port == 1) { // Check if port 1 is active
					oled_blit_P(laptop64x64, 64, 64, 32, 0, illuminationTime); // Show laptop image on OLED screen
				} else if (kvmpConfig.Device.Port == 2) { // Check if port 2 is active
					oled_blit_P(raspi64x64, 64, 64, 32, 0, illuminationTime); // Show Raspberry Pi image on OLED screen
				} else if (kvmpConfig.Device.Port == 3) { // Check if port 3 is active
					oled_blit_P(pc64x64, 64, 64, 32, 0, illuminationTime); // Show PC image on OLED screen
				} else {
					// Write content on the OLED screen
					oled_write("Undefined device image", false); 				
					// Flash error pattern on MacroPad
					defer_exec(1, deferred_error_flash_callback, &errorColor);
					return;
				}
				// Write the port number for the device
				oled_set_cursor(0, 0);  // Position the OLED cursor at the top left of the screen
				snprintf(line1, sizeof(line1), "Port:%d", kvmpConfig.Device.Port); // Insert content into line1
				oled_write(line1, false);  // Write line1 to the OLED screen
				
				// Write the target KVMP's primary function (as a footer)
				oled_set_cursor(16, 0);   // Position the OLED cursor at the top right of the screen
				if (kvmpConfig.Name == kvmp1Config.Name) {
					oled_write("Prime", false); // Write content on the OLED screen
				} else {
					oled_write("Touch", false); // Write content on the OLED screen
				}
			} else {
				// Write the port number for the device
				oled_set_cursor(0, 0);  // Position the OLED cursor at the top left of the screen
				snprintf(line1, sizeof(line1), "Port:?"); // Insert content into line1
				oled_write_ln(line1, false);  // Write line1 and include newline
				
				// Write the target KVMP's primary function (as a footer)
				oled_set_cursor(16, 0);   // Position the OLED cursor at the top right of the screen
				if (kvmpConfig.Name == kvmp1Config.Name) {
					oled_write("Prime", false); // Write content on the OLED screen
				} else {
					oled_write("Touch", false); // Write content on the OLED screen
				}
			}
			break;
			
		case ALL:
			if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all focus is actually on one device
				kvmp_config_report(kvmpConfig, DEVICE, illuminationTime); // Report selected port (device) configuration on OLED screen (with auto-clear)
				return;
			}
			
			// Write the target KVMP's primary function (as primary header)
			oled_set_cursor(8, 0);   // Position the OLED cursor at the bottom left of the screen
			oled_write("Prime", false); // Write content (KVMP primary function) on the OLED screen
			
			// Write the seconday header
			oled_set_cursor(0, 1);  // Position the OLED cursor at the top left of the screen
			snprintf(line1, sizeof(line1), "  KVM    USB    Audio"); // Insert content into line1
			oled_write_ln(line1, false);  // Write line1 and include newline
			
			// Write the port number for each device
			oled_set_cursor(0, 2);  // Position the OLED cursor at the top left of the screen
			char kvm_s[12], usb_s[12], aud_s[12]; 
			snprintf(line1, sizeof(line1), "   %s      %s       %s",  // Insert content into line1
				format_port_output(kvmpConfig.KVM.Port,   kvm_s, sizeof kvm_s),
				format_port_output(kvmpConfig.Usb.Port,   usb_s, sizeof usb_s),
				format_port_output(kvmpConfig.Audio.Port, aud_s, sizeof aud_s));
			oled_write_ln(line1, false);  // Write line1 and include newline
			
			// Write the device images
			oled_blit_P(image40x40_table[kvmpConfig.KVM.Port-1], 40, 40, 0, 24, illuminationTime); // Show which device has KVM focus on the OLED screen
			oled_blit_P(image40x40_table[kvmpConfig.Usb.Port-1], 40, 40, 44, 24, illuminationTime); // Show which device has USB hub focus on the OLED screen
			oled_blit_P(image40x40_table[kvmpConfig.Audio.Port-1], 40, 40, 89, 24, illuminationTime); // Show which device has Audio focus on the OLED screen
			
			break;
			
		default:
			// Write content on the OLED screen
			oled_write("Undefined case", false); 				
			// Flash error pattern on MacroPad
			defer_exec(1, deferred_error_flash_callback, &errorColor);
			return;
	}
	
	// Light the keys to display the current KVMP focus
	if (kvmpConfig.Device.Port > 0) {
		light_led_for(kvmpConfig.Device.Port, kvmpConfig.Device.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected port for a period of time (they will auto-off)
	}
	if (kvmpConfig.Name == kvmp1Config.Name) { // Check if inputted kvmpConfig is actually kvmp1Config
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
	
	if (oled_off_tok != INVALID_DEFERRED_TOKEN) { // Check if a valid deferral token already exists
		extend_deferred_exec(oled_off_tok, ms); // Extend the existing pending execution relative to "now"
	} else {
		oled_off_tok = defer_exec(ms, oled_off_cb, NULL); // Schedule a fresh deferral
	}
}

// Method to change ports on the ATEN CS1924 or CS1824 KVMP switches
void change_KVMP_port(KvmpConfig kvmpConfig, int portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	
	kvmpConfig.Device.Port = kvmpConfig.KVM.Port = kvmpConfig.Usb.Port = kvmpConfig.Audio.Port = ledNumber; // Update kvmpConfig to reflect the port change
	if (kvmpConfig.Name == kvmp1Config.Name) { // Check if inputted kvmpConfig is actually kvmp1Config
		kvmp1Config = kvmpConfig; // Update kvmp1Config
	} else {
		kvmp2Config = kvmpConfig; // Update kvmp2Config
	}
	kvmp_config_report(kvmpConfig, DEVICE, illuminationTime); // Report KVMP port configuration on OLED screen (with auto-clear)
	
	// Send series of key taps (macro) to focus on target port on the ATEN CS1924/CS1824 KVMP switch
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(portHotkey, keyPressDelay); // Send the keycode corresponding to the target port number
	tap_code(KC_ENTER);
}

// Method to change only the KVM (keyboard, video, mouse) focus on the ATEN CS1924/CS1824 KVMP switch to target port
static void change_KVM(KvmpConfig kvmpConfig, uint16_t portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.KVM.Port = ledNumber-3; // Update kvmpConfig to reflect the KVM focus change
	if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all elements are focusing on the same port
		kvmpConfig.Device.Port = kvmpConfig.KVM.Port; // Update kvmpConfig.Device.Port to align with the focus of all other elements
	}
	else {
		kvmpConfig.Device.Port = -1; // Update kvmpConfig.Device.Port to indicate that its value is not set
	}
	if (kvmpConfig.Name == kvmp1Config.Name) { // Check if inputted kvmpConfig is actually kvmp1Config
		kvmp1Config = kvmpConfig; // Update kvmp1Config
	} else {
		kvmp2Config = kvmpConfig; // Update kvmp2Config
	}
	kvmp_config_report(kvmpConfig, ALL, illuminationTime); // Report KVMP port or focus configuration for all devices on OLED screen (with auto-clear)
	///kvmp_config_report(kvmpConfig, KVM, illuminationTime); // Report KVMP port or focus configuration on OLED screen (with auto-clear)
	///light_led_for(ledNumber, kvmpConfig.KVM.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected KVM-focused port for a period of time (they will auto-off)
	
	// Send series of key taps (macro) to direct the ATEN CS1924/CS1824 KVMP switch's KVM to focus on a target port
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(portHotkey, keyPressDelay); // Send the keycode corresponding to the target port number
	tap_code_delay(KC_K, keyPressDelay); // Send modifier keycode to trigger a switch in the KVM only
	tap_code(KC_ENTER);
}

// Method to change only the usb hub focus on the ATEN CS1924/CS1824 KVMP switch to target port
static void change_usb(KvmpConfig kvmpConfig, uint16_t portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.Usb.Port = ledNumber-6; // Update kvmpConfig to reflect the USB focus change
	if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all elements are focusing on the same port
		kvmpConfig.Device.Port = kvmpConfig.Usb.Port; // Update kvmpConfig.Device.Port to align with the focus of all other elements
	}
	else {
		kvmpConfig.Device.Port = -1; // Update kvmpConfig.Device.Port to indicate that its value is not set
	}
	if (kvmpConfig.Name == kvmp1Config.Name) { // Check if inputted kvmpConfig is actually kvmp1Config
		kvmp1Config = kvmpConfig; // Update kvmp1Config
	} else {
		kvmp2Config = kvmpConfig; // Update kvmp2Config
	}
	kvmp_config_report(kvmpConfig, ALL, illuminationTime); // Report KVMP port or focus configuration for all devices on OLED screen (with auto-clear)
	///kvmp_config_report(kvmpConfig, USBHUB, illuminationTime); // Report port or focus configuration on OLED screen (with auto-clear)
	///light_led_for(ledNumber, kvmpConfig.Usb.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected USB-focused port for a period of time (they will auto-off)	
	
	// Send series of key taps (macro) to direct the ATEN CS1924/CS1824 KVMP switch's USB hub to focus on a target port
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(portHotkey, keyPressDelay); // Send the keycode corresponding to the target port number
	tap_code_delay(KC_U, keyPressDelay); // Send modifier keycode to trigger a switch in the USB hub only
	tap_code(KC_ENTER);
}

// Method to change only the audio focus on the ATEN CS1924/CS1824 KVMP switch to target port
static void change_audio(KvmpConfig kvmpConfig, uint16_t portHotkey, int ledNumber)
{
	all_leds_off_noeeprom(); // Turn off all LEDs
	kvmpConfig.Audio.Port = ledNumber-9; // Update kvmpConfig to reflect the Audio focus change
	if (kvmpConfig.KVM.Port == kvmpConfig.Usb.Port && kvmpConfig.Usb.Port == kvmpConfig.Audio.Port) { // Check if all elements are focusing on the same port
		kvmpConfig.Device.Port = kvmpConfig.Audio.Port; // Update kvmpConfig.Device.Port to align with the focus of all other elements
	}
	else {
		kvmpConfig.Device.Port = -1; // Update kvmpConfig.Device.Port to indicate that its value is not set
	}
	if (kvmpConfig.Name == kvmp1Config.Name) { // Check if inputted kvmpConfig is actually kvmp1Config
		kvmp1Config = kvmpConfig; // Update kvmp1Config
	} else {
		kvmp2Config = kvmpConfig; // Update kvmp2Config
	}
	kvmp_config_report(kvmpConfig, ALL, illuminationTime); // Report KVMP port or focus configuration for all devices on OLED screen (with auto-clear)
	///kvmp_config_report(kvmpConfig, AUDIO, illuminationTime); // Report KVMP port or focus configuration on OLED screen (with auto-clear)
	///light_led_for(ledNumber, kvmpConfig.Audio.Color, (uint16_t)illuminationTime); // Turn on keypad LED for selected audio-focused port for a period of time (they will auto-off)
	
	// Send series of key taps (macro) to direct the ATEN CS1924/CS1824 KVMP switch's audio to focus on a target port
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(KC_SCROLL_LOCK, keyPressDelay);
	tap_code_delay(portHotkey, keyPressDelay); // Send the keycode corresponding to the target port number
	tap_code_delay(KC_S, keyPressDelay); // Send modifier keycode to trigger a switch in the audio only
	tap_code(KC_ENTER);
}

// |----------------------|
// | Key Press Processing |
// |----------------------|

// Method run whenever a key is pressed or released
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	uint16_t portHotkey = 0;
	uint16_t ledNumber = 0;
    switch (keycode) {
	/* case PORT_CHECK:
        if (record->event.pressed) { // When keycode PORT_CHECK is pressed
			kvmp1_check(); // Call method to report current KVMP switch configuration
        } else {
            // When keycode PORT_CHECK is released
        }
        return true; */
		
    /* case PORT_1:
        if (record->event.pressed) { // When keycode PORT_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 1; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_KVMP_port(portHotkey, ledNumber); // Call method to change ports on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode PORT_1 is released
        }
        break; */

    /* case PORT_2:
        if (record->event.pressed) { // When keycode PORT_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 2; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_KVMP_port(portHotkey, ledNumber); // Call method to change ports on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode PORT_2 is released
        }
        break; */
		
    /* case PORT_3:
        if (record->event.pressed) { // When keycode PORT_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 3; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_KVMP_port(portHotkey, ledNumber); // Call method to change ports on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode PORT_3 is released
        }
        break; */
		
    case KVM_1:
        if (record->event.pressed) { // When keycode KVM_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 4; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_KVM(kvmp1Config, portHotkey, ledNumber); // Call method to change KVM focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode KVM_1 is released
        }
        break;
		
	case KVM_2:
        if (record->event.pressed) { // When keycode KVM_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 5; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_KVM(kvmp1Config, portHotkey, ledNumber); // Call method to change KVM focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode KVM_2 is released
        }
        break;
		
	case KVM_3:
        if (record->event.pressed) { // When keycode KVM_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 6; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_KVM(kvmp1Config, portHotkey, ledNumber); // Call method to change KVM focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode KVM_3 is released
        }
        break;
		
	case USB_1:
        if (record->event.pressed) { // When keycode USB_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 7; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_usb(kvmp1Config, portHotkey, ledNumber); // Call method to change USB hub focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode USB_1 is released
        }
        break;
		
	case USB_2:
        if (record->event.pressed) { // When keycode USB_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 8; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_usb(kvmp1Config, portHotkey, ledNumber); // Call method to change USB hub focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode USB_2 is released
        }
        break;
		
	case USB_3:
        if (record->event.pressed) { // When keycode USB_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 9; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_usb(kvmp1Config, portHotkey, ledNumber); // Call method to change USB hub focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode USB_3 is released
        }
        break;
		
	case AUDIO_1:
        if (record->event.pressed) { // When keycode AUDIO_1 is pressed
			portHotkey = KC_1; // Store target portHotkey
			ledNumber = 10; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_audio(kvmp1Config, portHotkey, ledNumber); // Call method to change audio focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode AUDIO_1 is released
        }
        break;
		
	case AUDIO_2:
        if (record->event.pressed) { // When keycode AUDIO_2 is pressed
			portHotkey = KC_2; // Store target portHotkey
			ledNumber = 11; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_audio(kvmp1Config, portHotkey, ledNumber); // Call method to change audio focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode AUDIO_2 is released
        }
        break;

	case AUDIO_3:
        if (record->event.pressed) { // When keycode AUDIO_3 is pressed
			portHotkey = KC_3; // Store target portHotkey
			ledNumber = 12; // Store LED number corresponding to pressed key
			change_KM_port(1); // Call method to change ports on the ATEN CS62KM KM switch (if needed)
			change_audio(kvmp1Config, portHotkey, ledNumber); // Call method to change audio focus on the ATEN CS1924 or CS1824 KVMP switch
        } else {
            // When keycode AUDIO_3 is released
        }
        break;
    }
    return true;
}

// Method for reporting current configuration on the ATEN CS1924 KVMP switch
void kvmp1_check(void) {	
	// Report configuration status on OLED
	all_leds_off_noeeprom(); // Turn off all key LEDs
	if (kvmp1Config.KVM.Port == kvmp1Config.Usb.Port && kvmp1Config.Usb.Port == kvmp1Config.Audio.Port) { // Check if KVM, USB, and Audio focus on the ATEN CS1924/CS1824 KVMP switch are all on the same port
		kvmp_config_report(kvmp1Config, DEVICE, illuminationTime); // Report selected KVMP port (device) on OLED screen (with auto-clear)
	} else {
		kvmp_config_report(kvmp1Config, ALL, illuminationTime); // Report KVMP port or focus configuration for all devices on OLED screen (with auto-clear)
	}
	
	// Trigger error flashing if needed
	int inputCheck = kvmp1Config.Device.Port + kvmp1Config.KVM.Port + kvmp1Config.Usb.Port + kvmp1Config.Audio.Port;
	if (inputCheck == -4) { // Check if MacroPad hasn't been used to send port or focus selection input to ATEN CS1924 KVMP switch	
		// Flash error pattern on MacroPad
        defer_exec(1, deferred_error_flash_callback, &errorColor);
	}
}

// Method for reporting current configuration on the CS1824 KVMP switch
void kvmp2_check(void) {
	// Report configuration status on OLED
	all_leds_off_noeeprom(); // Turn off all key LEDs
	kvmp_config_report(kvmp2Config, DEVICE, illuminationTime); // Report selected KVMP port (device) on OLED screen (with auto-clear)
	
	// Trigger error flashing if needed
	if (kvmp2Config.Device.Port == -1) { // Check if MacroPad hasn't been used to send port or focus selection input to the ATEN CS1824 KVMP switch
		// Flash error pattern on MacroPad
        defer_exec(1, deferred_error_flash_callback, &errorColor);
	}
}

// |-----------------|
// | Host Sleep/Wake |
// |-----------------|

// Method run when the host PC suspends (e.g., sleep)
void suspend_power_down_user(void) {
    ///rgblight_sethsv_noeeprom(HSV_BLUE); // Turn all LEDs on to Blue (debug)
	all_leds_off_noeeprom(); // Turn off all LEDs
    oled_off(); // Clear the OLED screen
	
	/* // Play a shutdown audio song
	PLAY_SONG(goodbye_song); */
}

// Method run when the host PC resumes (e.g., wakes up)
void suspend_wakeup_init_user(void) {
	if (oled_off_tok == INVALID_DEFERRED_TOKEN) { // Check if there is no oled_off_tok active
		keyboard_post_init_user(); // Call method to initialize the MacroPad
	} /* else {
		rgblight_sethsv_noeeprom(HSV_RED); // Turn all LEDs on to Red (debug)
	} */
}

// ---------------------------------------------------------------------------------------------------------

// *************
// *  ARCHIVE  *
// *************

// empty