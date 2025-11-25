# Adafruit MacroPad RP2040
<img width="574" height="431" alt="image" src="https://github.com/user-attachments/assets/769a5053-d559-46b8-8a90-bd025da6390b" />

## What Is This?
This project includes QMK files for various keymaps for the [Adafruit MacroPad RP2040](https://learn.adafruit.com/adafruit-macropad-rp2040), which includes 12 RGB LED key switches, a pressable rotary encoder, an OLED screen, and audio output capability. 

Jump to the latest release here (LINK!!!).

## Key Maps
### Layout
<img width="575" height="712" alt="image" src="https://github.com/user-attachments/assets/625c252d-d6e4-4647-9010-c3e3ee2dcb0d" />

### UltraKvmpSwitch
Configuration for simultaneously controlling the ATEN [CS62KM](https://www.aten.com/us/en/products/kvm/cable-kvm-switches/cs62km/) KM switch _as well as_ the ATEN [CS1824](https://www.aten.com/us/en/products/kvm/desktop-kvm-switches/cs1824/) _and_ [CS1924](https://www.aten.com/us/en/products/kvm/desktop-kvm-switches/cs1924/) KVMP switches. 
  * The MacroPad connects to the CS62KM, which is then connected to both the CS1824 and CS1924
  * This enables the MacroPad to control both the CS1824 and CS1924 by using the CS62KM as an intermediary

Jump to the files [here](https://github.com/Exergist/Adafruit-MacroPad-RP2040/tree/main/QMK/UltraKvmpSwitch).
<details>
<summary><b>Keymap</b></summary>
 
 * OLED Screen = Displays configuration information
 * Encoder (ENCODER_DANCE)
   * Single-Tap = Illuminate keys corresponding to the current configuration for the CS1924 KVMP switch
   * Press-Hold = Illuminate key corresponding to the current configuration for the CS1824 KVMP switch
   * Clockwise Rotation = Volume Up
   * Counter-Clockwise Rotation = Volume Down
 * Button1 (PORT1_DANCE)
   * Single-Tap = Change to port 1 on the CS1924
   * Press-Hold = Change to port 1 on the CS1824
  * Button2 (PORT2_DANCE)
   * Single-Tap = Change to port 2 on the CS1924
   * Press-Hold = Change to port 2 on the CS1824
  * Button3 (PORT3_DANCE)
   * Single-Tap = Change to port 3 on the CS1924
   * Press-Hold = Change to port 3 on the CS1824
 * Button4 (KVM_1) = Change KVM focus on the CS1924 to port 1
 * Button5 (KVM_2) = Change KVM focus on the CS1924 to port 2
 * Button6 (KVM_3) = Change KVM focus on the CS1924 to port 3
 * Button7 (USB_1) = Change USB hub focus on the CS1924 to port 1
 * Button8 (USB_2) = Change USB hub focus on the CS1924 to port 2
 * Button9 (USB_3) = Change USB hub focus on the CS1924 to port 3
 * Button10 (AUDIO_1) = Change Audio focus on the CS1924 to port 1
 * Button11 (AUDIO_2) = Change Audio focus on the CS1924 to port 2
 * Button12 (AUDIO_3) = Change Audio focus on the CS1924 to port 3
</details>

### SuperKvmpSwitch
Configuration for the ATEN [CS1924](https://www.aten.com/us/en/products/kvm/desktop-kvm-switches/cs1924/) _or_ [CS1824](https://www.aten.com/us/en/products/kvm/desktop-kvm-switches/cs1824/) KVMP switch.  
  * In this configuration the MacroPad connects directly to the CS1924 or CS1824 KVMP switch

Jump to the files [here](https://github.com/Exergist/Adafruit-MacroPad-RP2040/tree/main/QMK/SuperKvmpSwitch).
<details>
<summary><b>Keymap</b></summary>
 
 * OLED Screen = Displays configuration information
 * Encoder
   * Single-Tap = PORT_CHECK = Illuminate keys corresponding to the current configuration for the CS1824/CS1924
   * Clockwise Rotation = Volume Up
   * Counter-Clockwise Rotation = Volume Down
 * Button1 (PORT_1) = Change to port 1 on the CS1824/CS1924
 * Button2 (PORT_2) = Change to port 2 on the CS1824/CS1924
 * Button3 (PORT_3) = Change to port 3 on the CS1824/CS1924
 * Button4 (KVM_1) = Change KVM focus on the CS1824/CS1924 to port 1
 * Button5 (KVM_2) = Change KVM focus on the CS1824/CS1924 to port 2
 * Button6 (KVM_3) = Change KVM focus on the CS1824/CS1924 to port 3
 * Button7 (USB_1) = Change USB hub focus on the CS1824/CS1924 to port 1
 * Button8 (USB_2) = Change USB hub focus on the CS1824/CS1924 to port 2
 * Button9 (USB_3) = Change USB hub focus on the CS1824/CS1924 to port 3
 * Button10 (AUDIO_1) = Change Audio focus on the CS1824/CS1924 to port 1
 * Button11 (AUDIO_2) = Change Audio focus on the CS1824/CS1924 to port 2
 * Button12 (AUDIO_3) = Change Audio focus on the CS1824/CS1924 to port 3
</details>

## How to Build and Install the Firmware?
### Creating the Firmware
<details>
<summary> <b>QMK Configurator Method</b> </summary>

 1. Load the [MacroPad RP2040 default configuration](https://config.qmk.fm/#/adafruit/macropad/LAYOUT) in the QMK Configurator.
 2. Select the functions for the buttons and click compile.
    
    <img width="883" height="764" alt="image" src="https://github.com/user-attachments/assets/978bf79f-de0d-4af6-a520-38217a9c30d9" />

 3. Once the compiling is complete, download and save the firmware uf2 file.
</details>
<details>
<summary> <b>QMK Code-Only Method (Recommended) </b> </summary>
<details>
<summary> <i>General</i> </summary>
 
 1. Go through [The QMK Tutorial](https://docs.qmk.fm/#/newbs). To get started, create a new keymap project within QMK MSYS environment by running `qmk new-keymap -kb [keyboard name] -km [name of project]`
    * Example: `qmk new-keymap -kb adafruit/macropad -km UltraKvmpSwitch`
 3. Create the desired keyboard behavior by building content in the _keymap.c_ file.
 4. Add (or copy from other QMK MacroPad RP2040 efforts) _config.h_ and _rules.mk_ files if needed (most likely YES) into the same location as the _keymap.c_ file.
 5. When _keymap.c_ and (if applicable) _config.h_ and _rules.mk_ are ready, go back to QMK MSYS and run `qmk compile -j 0 -kb [keyboard name] -km [name of project]`.
    * Example: `qmk compile -j 0 -kb adafruit/macropad -km UltraKvmpSwitch`
 6. The outputted firmware (uf2 file) is located at `...qmk_firmware\.build\[keyboard_name]_[project_name].uf2`
    * Example: `...qmk_firmware\.build\adafruit_macropad_UltraKvmpSwitch.uf2`
</details>
 <details>
<summary> <i>Creating Images for the OLED Screen</i> </summary>

  1. Create an image (such as a *.png) you want to display on the OLED screen. Note that for best results the image should have white foreground coloring and black background coloring. 
  2. Convert the image to a C byte array using a tool such as [image2cpp](https://javl.github.io/image2cpp/). Here are some detailed instructions when using image2cpp:
     * The "Canvas Size" is the extent of the image as displayed on the OLED screen. Ideally the canvas width and height are multiples of 8 (e.g., 32x32, 64x64).
     * Background Color = Black
     * Dithering = Binary (though other types will likely work fine)
     * Brightness/Alpha Threshold = 128
     * Scaling = Scale to fit, keeping proportions
     * Center Image = Horizontally and Vertically
     * Code Output Format = Plain bytes
     * Draw Mode = Vertical - 1 bit per pixel
  3. The image2cpp tool provides a preview of what the image will look like for the provided Canvas Size.
  4. Press the "Generate code" button, and a byte array corresponding to the preview image will be created.
  5. Leverage this created byte array in the `keymap.c` file (see [Source](https://github.com/Exergist/Adafruit-MacroPad-RP2040/blob/main/QMK/UltraKvmpSwitch/Source/keymap.c) for example usage).
</details>
</details>



<details>
<summary> <b>CircuitPython Method</b> </summary>

Adafruit provides a [comprehensive guide](https://learn.adafruit.com/adafruit-macropad-rp2040/circuitpython) that walks through using CircuitPython to build firmware for the MacroPad RP2040. Note that this project currently focuses on using QMK for building the firmware.
</details>

### Installing the Firmware
<details>
<summary> <b>Firmware Drag-and-Drop Method</b> </summary>

 1. Plug the Adafruit MacroPad RP2040 into the computer.
 2. Press and hold down on the encoder, and then press the reset button (located to the left of the OLED screen on the side of the board).
    
    <img width="493" height="440" alt="image" src="https://github.com/user-attachments/assets/39ce9909-b5a6-49b1-8311-48b0f20a5684" />

 3. The MacroPad should enter bootloader mode and a new USB storage device called `RPI-RP2` should be available on the computer.
 4. Drag and drop the desired firmware uf2 file into the RPI-RP2 device.
 5. The MacroPad should auto-restart and launch with the new firmware.

</details>

## Notes
* The ATEN CS62KM does not support media keys, so rotating the MacroPad encoder when it is connected to the CS62KM does not change the PC volume
* My MacroPad RP2040 was ordered through [Adafruit](https://www.adafruit.com/product/5128) in December 2021
* Additional MacroPad RP2040 resources may be found on [Adafruit's website](https://learn.adafruit.com/adafruit-macropad-rp2040)
* Additional QMK resources are compiled in the [QMK Syllabus](https://docs.qmk.fm/#/syllabus)

## Acknowledgements
* [QMK](https://qmk.fm/about) is developed and maintained by Jack Humbert of OLKB with contributions from the community (and Hasu)
* Keymap header ASCII art by [patorjk](https://patorjk.com/software/taag/)
* [ForsakenRei](https://github.com/ForsakenRei) for his [RP2040-MacroPad](https://github.com/ForsakenRei/RP2040-MacroPad/tree/main) project, which was a useful reference
* "Laptop" image created by Dong Gyu Yang from [Noun Project](https://thenounproject.com/icon/laptop-8086742/)
