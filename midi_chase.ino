#include "FastLED.h"
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);


#define NUM_LEDS 200
#define DATA_PIN 7
#define CLOCK_PIN 8
#define sleep_action false
#define space 60.0
#define max_midi 128.0

int step_size = exp((log(space)/3.0));
int b_max = space;
int g_max = (space - (space/3.0));
int r_max = (space - ((space/3.0)*2));
bool flashOn = false;

CRGB leds[NUM_LEDS];
CRGB tertiary[NUM_LEDS];
int now = millis();
int power = 0;
int noteOn = 0;
int r = 0;
int g = 0;
int b = 0;

void setup() {
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  leds[0] = CRGB::Black;
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(9600);
}

void rgb(int someval) {
  r = 0;
  g = 0;
  b = 0;
    
  if (someval > space) {
    someval = someval - (someval - space);
  }
  
  if (someval < r_max) {
    r = map(someval,0,r_max,0,255);
  }
  if ((someval >= r_max) && (someval <= g_max)) {
    b = map(someval-r_max,0,r_max,0,255);
    r = 255 - b;
  }
  if (someval >= g_max) {
    r = map(someval-g_max,0,r_max,0,255);
    g = 255 - r;
  }
  if (someval == space) {
    b = 255;
  }
  
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {

  if (channel == 10) {
    cache();
    flashOn = true;
    flash(pitch);
  }
  else {
    rgb(pitch);
    noteOn = 1;
  }
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (channel == 10) {
    de_cache();
    flashOn = false;
  }
  else {
    noteOn = 0;
  }
}

void blackout() {
  for (int i = 0; i < NUM_LEDS - 1; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void shift() {
  FastLED.show(); 
  for (int i = NUM_LEDS-1; i>=1; i--) {
    leds[i] = leds[i-1];
 }
}

void flash(int pitch) {
  for (int i=1; i < NUM_LEDS; i++) {
    if (pitch == 36) {
      leds[i] = CRGB::Red;
    }
    if (pitch == 40) {
      leds[i] = CRGB::White;
    }
    if (pitch == 37) {
      leds[i].r=255;
      leds[i].g=20;
      leds[i].b=100;
    }
  }
  FastLED.show();
}

void cache() {
  for (int i = 0; i<NUM_LEDS; i++) {
    tertiary[i] = leds[i];
  }
}

void de_cache() {
  for (int i = 0; i<NUM_LEDS; i++) {
    leds[i] = tertiary[i];
  }
}

void loop() {
  if ((millis() % 25 == 0) && (flashOn == false)) {
    shift();
  }
  
  if (noteOn == 1) {
    leds[1].r = r;
    leds[1].g = g;
    leds[1].b = b;
  }  
  MIDI.read();
}
