# PianoLED
Control an LED strip with MIDI messages on an Arduino.

This Arduino sketch takes MIDI messages from the serial port, and maps them to individual LEDs in the strip,
so a MIDI ON message effectively turns on an LED, and MIDI OFF fades it out.

There are implementations for 7 lighting modes.

## Configuration
**The sketch needs the following:**
  - A MIDI cable connected to pin RX0 via optoisolator circuit
  - A 2-pin button connected to pin 5 and ground (*INPUT_PULLDOWN*)
  - A potentiometer connected to pin A5
  - An optional info LED to pin 13
  - An LED Strip supported by the [FastLED library](https://github.com/FastLED/FastLED).
  - A piano or anything that generates MIDI data
  
**Customizable parameters:**
 - The LED strip model, color order and number of LEDs can be configured at the top of the file (default is a strip of 74 _WS2812B_ LEDs, color order _GRB_, suitable for a regular 8-octave digital piano.
 - Note fade out duration, sustain pedal strength
 - Color HSV values, palettes (right now there are 6 palettes that can be navigated through with the potentiometer)
 - New modes can be added fairly easily.
 
## How to use
Use the button to toggle between 6 modes:
  - **Mode 0**: Off
  - **Mode 1**: Passive (cycle between different LED animations, e.g. rainbow, trail, sparkles)
  - **Mode 2**: Basic (notes light up with a fixed color - potentiometer changes hue)
  - **Mode 3**: Alternate Basic (same as mode 2, but with less saturated colors)
  - **Mode 4**: Palette (color depends on the note pitch, follows a gradient pattern - potentiometer navigates through 6 palettes)
  - **Mode 5**: Velocity Palette (same as mode 4, but color depends on how hard the key is pressed - 2 palettes)
  - **Mode 6**: Hue Cycle (notes light up with a fixed color that changes over time)
  - **Mode 7**: Echo/Burst (when a note is played, surrounding LEDs light up in a 'burst' fashion)


Enjoy!
