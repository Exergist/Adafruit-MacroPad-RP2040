# Adafruit MacroPad RP2040
<img width="574" height="431" alt="image" src="https://github.com/user-attachments/assets/769a5053-d559-46b8-8a90-bd025da6390b" />

## What Is This?
This project includes QMK files for various keymaps for the [Adafruit MacroPad RP2040](https://learn.adafruit.com/adafruit-macropad-rp2040), which includes 12 RGB LED key switches, a rotary encoder, and an OLED screen. 

Jump to the latest release here (LINK!!!).

## Key Maps
### Layout
<img width="575" height="712" alt="image" src="https://github.com/user-attachments/assets/625c252d-d6e4-4647-9010-c3e3ee2dcb0d" />

### SuperKvmpSwitch
Control device for the [ATEN CS1924](https://www.aten.com/us/en/products/kvm/desktop-kvm-switches/cs1924/) 4-port KVMP switch. Jump to the files [here](https://github.com/Exergist/Adafruit-MacroPad-RP2040/tree/main/QMK/SuperDisplaySwitch). 
<details>
<summary><b>Keymap</b></summary>
 
  * OLED Screen = WIP
 * Encoder
   * Single-Tap = Illuminate keys corresponding to the CS1924 KVMP's current configuration
   * Clockwise Rotation = Volume Up
   * Counter-Clockwise Rotation = Volume Down
 * Button 1 = WIP
 * Button 2 = WIP
 * Button 3 = WIP
 * Button 4 = WIP
 * Button 5 = WIP
 * Button 6 = WIP
 * Button 7 = WIP
 * Button 8 = WIP
 * Button 9 = WIP
 * Button 10 = WIP
 * Button 11 = WIP
 * Button 12 = WIP
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
<summary> <b>QMK Code-Only Method</b> </summary>

 1. Go through [The QMK Tutorial](https://docs.qmk.fm/#/newbs). To get started, create a new keymap project within QMK MSYS environment by running `qmk new-keymap -kb [keyboard name] -km [name of project]`
    * Example: `qmk new-keymap -kb adafruit/macropad -km SuperKvmpSwitch`
 3. Create the desired keyboard behavior by building content in the _keymap.c_ file.
 4. Add (or copy from other QMK MacroPad RP2040 efforts) _config.h_ and _rules.mk_ files if needed (most likely YES) into the same location as the _keymap.c_ file.
 5. When _keymap.c_ and (if applicable) _config.h_ and _rules.mk_ are ready, go back to QMK MSYS and run `qmk compile -j 0 -kb [keyboard name] -km [name of project]`.
    * Example: `qmk compile -j 0 -kb adafruit/macropad -km SuperKvmpSwitch`
 6. The outputted firmware (uf2 file) is located at `...qmk_firmware\.build\[keyboard_name]_[project_name].uf2`
    * Example: `...qmk_firmware\.build\adafruit_macropad_SuperKvmpSwitch.uf2`
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
    [Show image of the MacroPad and highlight the locations of the encoder and reset button]
 3. The MacroPad should enter bootloader mode and a new device should be available on the computer (???????).
    [Show image of the new device]
 5. Drag and drop the desired firmware uf2 file into the (???????) device.
 6. The MacroPad should auto-restart and launch with the new firmware.

</details>

## Notes
 * My MacroPad RP2040 was ordered through [Adafruit](https://www.adafruit.com/product/5128) in December 2021. 
 * Additional MacroPad RP2040 resources may be found on [Adafruit's website](https://learn.adafruit.com/adafruit-macropad-rp2040).
 * Additional QMK resources are compiled in the [QMK Syllabus](https://docs.qmk.fm/#/syllabus).


## Acknowledgements
* [QMK](https://qmk.fm/about) is developed and maintained by Jack Humbert of OLKB with contributions from the community (and Hasu).
* Keymap header ASCII art by [patorjk](https://patorjk.com/software/taag/).
* [ForsakenRei](https://github.com/ForsakenRei) for his [RP2040-MacroPad](https://github.com/ForsakenRei/RP2040-MacroPad/tree/main) project, which was a useful reference.
