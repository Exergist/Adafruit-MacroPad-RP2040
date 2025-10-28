# Adafruit MacroPad RP2040
<img width="574" height="431" alt="image" src="https://github.com/user-attachments/assets/769a5053-d559-46b8-8a90-bd025da6390b" />

## What Is This?
This project includes QMK files for various keymaps for the Adafruit MacroPad RP2040 with 12 RGB LED key switches, a rotary indented encoder, and an OLED screen. 

## Key Maps
### Layout
<img width="525" height="709" alt="image" src="https://github.com/user-attachments/assets/610a6c41-6475-478b-9a8f-e0b396c9ec2f" />

### SuperDisplaySwitch
Control device for the [ATEN CS1924](https://www.aten.com/us/en/products/kvm/desktop-kvm-switches/cs1924/) 4-port KVMP switch. Jump to the files here [***ADD THE LINK!!]. 
<details>
<summary><b>Keymap</b></summary>

 * Encoder
   * Single-Tap = Illuminate key corresponding to currently focused port (if available)
   * Clockwise Rotation = Volume Up
   * Counter-Clockwise Rotation = Volume Down
 * Button 1 = Switch to port 1
 * Button 2 = Switch to port 2
 * Button 3 = Switch to port 3
 * Button 4 = Switch to port 4
</details>

PLACEHOLDER FOR NOW
Use QMK MSYS to get rolling:
  * New project ==> `qmk new-keymap -kb adafruit/macropad -km SuperDisplaySwitch`
  * Compiling ==> `qmk compile -j 0 -kb adafruit/macropad -km SuperDisplaySwitch`
  * Firmware Hex output ==> `...qmk_firmware\.build`
Plug in Adafruit macropad into PC. Press down on the encoder and press the reset button to the left of the OLED screen. Macropad should enter bootloader mode and a new window should pop up in Windows.
Drag and drop a desired firmware uf2 file into the new RP1-RP2 window. The macropad should then auto-restart and launch with the new firmware.
