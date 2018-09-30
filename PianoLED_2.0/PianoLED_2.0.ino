// PIANO LED
// Nicolas de Ory 2017
// Programa que funciona con la tira de LEDs WS2812B y el teclado Yamaha CLP320
// MIRAR ISSUE GITHUB MIDI LIB
// Deberiamos comprobar available SRAM en void loop() para ver que pasa
//#define FASTLED_ALLOW_INTERRUPTS 0
#include <MIDI.h>
#include "FastLED.h"
#include <AltSoftSerial.h>

FASTLED_USING_NAMESPACE

#define button_pin  5
#define POTENTIOMETER_PIN 5 // analog 5

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    76
#define PEDAL_STRENGTH 30 // CHANGE?? TO VARIABLE
#define NO_PEDAL_STRENGTH 60
#define NOTE_HOLD_FADE 5

byte mode = 0;
CRGB leds[NUM_LEDS];
boolean onLeds[NUM_LEDS];

#define BRIGHTNESS          90
#define PASSIVE_FPS 120

// MODE 5 PALETTES

DEFINE_GRADIENT_PALETTE( GPVelocidad ) {
  0,     54,  67,  255,  
50,   54,  184,  255, 
100,   54,255,  161, 
145,   54,255,60,
235,   10, 255, 10,
255,   9, 255, 9 };

DEFINE_GRADIENT_PALETTE( GPHot ) {
    0,   200,  0,  0,
   55, 255,  0,  0,
  190, 255,255,  0,
  255, 255,255,255};


// MODE 4 PALETTES

DEFINE_GRADIENT_PALETTE( GP_Rainbow ) {
  0,     255,  0,  0,
43,   255,  102,  0,
81, 255, 204, 0,
122, 123, 255,  0,
166, 39,230,160,
229, 0, 255, 255,
245, 0,153,255,
255, 0, 153, 255 };

DEFINE_GRADIENT_PALETTE( GP_Tropical ) {
  40,     0, 176, 155,
  215, 150, 201, 6,
  255, 150, 202, 6};

DEFINE_GRADIENT_PALETTE( GP_PinkChampagne ) {
    0, 169, 25, 37,
  127, 182,117,149,
  255,  19,117,147 };

DEFINE_GRADIENT_PALETTE( GP_Emerald ) {
    0,   79, 142, 7,
  73, 88,196,  7,
  126, 88,195,  7,
  150, 206,237,138,
  255, 213,233,158};
  
DEFINE_GRADIENT_PALETTE( GP_JewelDragon ) {
   0,  11,  7, 13,
   35,  43, 20, 40,
   58,  91, 63, 95,
   81, 139, 88, 95,
  104, 140, 91, 87,
  127, 184,104,127,
  150, 215, 96, 83,
  173, 232, 97, 59,
  196, 208, 78, 27,
  219, 182, 42, 10,
  237, 150, 25,  3,
  255,  36,  5,  1};

DEFINE_GRADIENT_PALETTE( GP_FreshBlue ) {
   0,  52,150,80,
   60,  52,162,102,
  132,  11,111,138,
  200,  75,239,242,
  255,  75,238,220};


#define MODE4_PALETTE_COUNT 6

CRGBPalette16 mode4Palettes[] = {
  GP_Rainbow,
  GP_Tropical,
  GP_PinkChampagne,
  GP_Emerald,
  GP_JewelDragon,
  GP_FreshBlue
};

#define MODE5_PALETTE_COUNT 2

//CRGBPalette16 mode4Pal = GPRainbow;
CRGBPalette16 mode5Palettes[] = {
  GPVelocidad,
  GPHot
};


uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t customHue = 0;

uint8_t mode4PalIndex;
uint8_t mode5PalIndex;

struct MidiSettings : public midi::DefaultSettings
{
    static const bool UseRunningStatus = true; // My devices seem to be ok with it.
    static const unsigned SysExMaxSize = 2;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MidiSettings);
bool sustain;
bool DONT_FADE_NOTES = false;

void OnNoteOn(byte channel, byte pitch, byte velocity) {
  byte pitchcheck = constrain(pitch,19,107);
  byte velocitycheck = constrain(velocity,0,127);
  byte ld = map(pitchcheck,19,107,NUM_LEDS-2,1);
  onLeds[ld] = true;
   switch (mode) {
      case 2:
        leds[ld].setHSV(customHue, 150, 255);
        break;
       case 3:
        leds[ld].setHSV(customHue, 60, 255);
        break;
       case 4:
        leds[ld] = ColorFromPalette(mode4Palettes[mode4PalIndex],map(pitchcheck,19,107,0,240));
        break;
       case 5:
        leds[ld] = ColorFromPalette(mode5Palettes[mode5PalIndex],map(velocitycheck,0,127,0,240));
        break;
       case 6:
        leds[ld].setHSV(gHue, customHue, 255);
        break;
       case 7: // FADE AROUND NOTE
        int x=1;
        leds[ld].setHSV(customHue, 115, 255);
        for(int i = ld+1; i < NUM_LEDS; i++) {
          if(onLeds[i]) continue;
          if(255-x*45 <= 100) break;
          leds[i].setHSV(customHue, 110, 255-x*45);
          x++;
        }
        x = 1;
        for(int i = ld-1; i >= 0; i--) {
          if(onLeds[i]) continue;
          if(255-x*45 <= 100) break;
          leds[i].setHSV(customHue, 110, 255-x*45);
          x++;
        }
        break;
   }
}

void OnNoteOff(byte channel, byte pitch, byte velocity) { 
    byte pitchcheck2 = constrain(pitch,19,107);
    byte led = map(pitchcheck2,19,107,NUM_LEDS-2,1);
    onLeds[led] = false;

}

void OnControlChange(byte channel, byte number, byte value) { // This might be inexact== FALLO CON LAS LUCES
 if (number == 64) {
  if (value > 63) {
    sustain = true; 
  } else { 
  sustain = false; 
  }
 } else if (number == 67) {
  if (value > 63) {
    DONT_FADE_NOTES = true; 
  } else { 
  DONT_FADE_NOTES = false; 
  }
 }
}


void setup() {
  delay(2000); // Safety delay
  
  // MIDI SETUP
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  

  
  // LED SETUP
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  pinMode(13,OUTPUT); // DEBUG LED
  pinMode(button_pin, INPUT_PULLUP); // BUTTON

}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon};

//int lastAnalogRead;
int currAnalogRead;
void loop() {
 
 // PROCESS DIGITAL INPUT
 // Button
   if (digitalRead(button_pin) == LOW) {
      digitalWrite(13, HIGH);
      delay(500);
      if (mode == 7) { mode = 0; } else { mode++; }
   } else digitalWrite(13, LOW); 
 // Potentiometer
 currAnalogRead = analogRead(POTENTIOMETER_PIN);
  if(mode == 4) {
    int idx = constrain(floor(map(currAnalogRead,0,1023,0,MODE4_PALETTE_COUNT)),0,MODE4_PALETTE_COUNT-1);
    if(mode4PalIndex != idx) {
      digitalWrite(13, HIGH);
      mode4PalIndex = idx;
      // FILL PALETTE TO PREVIEW (in reverse) //
      int incr = floor(255/NUM_LEDS);
      int colorIndex = 255;
      for( uint16_t i = 0; i < NUM_LEDS; i++) { 
        leds[i] = ColorFromPalette(mode4Palettes[mode4PalIndex], colorIndex);
        colorIndex -= incr;
      }
      /////
      delay(100);
      digitalWrite(13, LOW);
    }
  } else if (mode == 5)
  {
    int idx = constrain(floor(map(currAnalogRead,0,1023,0,MODE5_PALETTE_COUNT)),0,MODE5_PALETTE_COUNT-1);
    if(mode5PalIndex != idx) {
      digitalWrite(13, HIGH);
      mode5PalIndex = idx;
      delay(100);
      digitalWrite(13, LOW);
    }
  } else {
    customHue = map(currAnalogRead,0,1023,0,255);
  }
  //lastAnalogRead = currAnalogRead;
 //////
 
 MIDI.read();
 
 if (mode == 1) { 
    gPatterns[gCurrentPatternNumber](); 
    FastLED.show(); 
    FastLED.delay(1000/PASSIVE_FPS); 
 }

EVERY_N_MILLISECONDS(20) { if (mode==1) { gHue++; } }
EVERY_N_MILLISECONDS(50) { if (mode==6) { gHue++; } }

EVERY_N_MILLISECONDS(40) {
  if (mode != 1) {
    for (byte i=0;i < NUM_LEDS; i++) { // THIS MIGHT NOT TURN OFF THE LEDS"!!
          if (leds[i].getAverageLight() == 0) {
              onLeds[i] = false;
              leds[i] = CRGB::Black;
        
          } else if (onLeds[i] == false) {
            if (sustain) {
              leds[i].fadeToBlackBy(PEDAL_STRENGTH);
            } else {
              leds[i].fadeToBlackBy(NO_PEDAL_STRENGTH);
            }
          } else {
            if(!DONT_FADE_NOTES)
              leds[i].fadeToBlackBy(NOTE_HOLD_FADE);
          }
    }
    FastLED.show();
  }
}
EVERY_N_SECONDS(20) { if (mode == 1) { nextPattern(); }}

}

// PASSIVE PATTERNS (MODE 0)
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(20);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}
