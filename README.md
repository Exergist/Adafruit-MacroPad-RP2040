# Adafruit-MacroPad-RP2040
Source, firmware, and related files for Adafruit Macropad projects


PLACEHOLDER FOR NOW
Use QMK MSYS to get rolling:
  * New project ==> `qmk new-keymap -kb adafruit/macropad -km SuperDisplaySwitch`
  * Compiling ==> `qmk compile -j 0 -kb adafruit/macropad -km SuperDisplaySwitch`
  * Firmware Hex output ==> `...qmk_firmware\.build`
Plug in Adafruit macropad into PC. Press down on the encoder and press the reset button to the left of the OLED screen. Macropad should enter bootloader mode and a new window should pop up in Windows.
Drag and drop a desired firmware uf2 file into the new RP1-RP2 window. The macropad should then auto-restart and launch with the new firmware.
