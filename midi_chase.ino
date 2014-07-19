#include "FastLED.h"
#include <MIDI.h>


MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);


#define NUM_LEDS 200
#define DATA_PIN 7
#define CLOCK_PIN 8
#define sleep_action false
#define space 60.0
#define max_midi 128.0
#define petal_size 200

int step_size = exp((log(space)/3.0));
int b_max = space;
int g_max = (space - (space/3.0));
int r_max = (space - ((space/3.0)*2));
bool flashOn = false;

CRGB leds[NUM_LEDS];
CRGB tertiary[NUM_LEDS];
unsigned long ontimes[NUM_LEDS];
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

void rgb(int pitch) {
  r = 0;
  g = 0;
  b = 0;
  
  if (pitch > 60) {
    int offset = (pitch % 60);
    pitch = 60 - offset;
  }
    
  if (pitch > space) {
    pitch = pitch - (pitch - space);
  }
  
  if (pitch < r_max) {
    r = map(pitch,0,r_max,0,255);
  }
  if ((pitch >= r_max) && (pitch <= g_max)) {
    b = map(pitch-r_max,0,r_max,0,255);
    r = 255 - b;
  }
  if (pitch >= g_max) {
    r = map(pitch-g_max,0,r_max,0,255);
    g = 255 - r;
  }
  if (pitch == space) {
    b = 255;
  }
  
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  Serial.println("Note on!");
      
  if (channel == 10) {
    if (flashOn == false) {
      cache();
    }
    flashOn = true;
    flash(pitch);
  }
  else {
    ontimes[pitch] = millis();
  }
  Serial.println(ontimes[pitch]);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  
  //if this is the drum pad
  if (channel == 10) {
    de_cache();
    flashOn = false;
  }
  else {
    ontimes[pitch] = 0;
  }
}

//turn off the strip
void blackout() {
  for (int i = 0; i < NUM_LEDS - 1; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

//shift every member of leds up
void shift() {
  FastLED.show(); 
  for (int i = NUM_LEDS-1; i>=1; i--) {
    leds[i].r = leds[i-1].r/1;
    leds[i].g = leds[i-1].g/1;
    leds[i].b = leds[i-1].b/1;
 }
}


//set every LED to on
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

//save the current state of the strip to an array
void cache() {
  for (int i = 0; i<NUM_LEDS; i++) {
    tertiary[i] = leds[i];
  }
}

//write the currently saved strip state back out to the strip from memory
void de_cache() {
  for (int i = 0; i<NUM_LEDS; i++) {
    leds[i] = tertiary[i];
  }
}

void flowerize() {
  for (int i = 0; i<NUM_LEDS; i+= petal_size) {
    leds[i] = CRGB::Black;
  }
}

int get_pitch() {
  int pitch = 0;
  int onkeys = 0;
  int total = 0;
  noteOn = 0;
  for (int i =0; i<NUM_LEDS; i++) {
    if (ontimes[i] > 0) {
      total = total + i;
      onkeys++;
    }
  }
  if (total > 0) {
    noteOn = 1;
    pitch = total/onkeys;
  }
  else {
    noteOn = 0;
  }
  return(pitch);
}

void show_keys() {
  for (int i = 0; i<NUM_LEDS; i++) {
    if (ontimes[i] > 0) {
      Serial.print(ontimes[i]);
      Serial.print("-");
      Serial.println();
    }
  }
}
  
  

void loop() {
  //shift the leds every 80ms, except when the flash buttons are being pressed, then skip it.
    
  if ((millis() % 85 == 0) && (flashOn == false)) {
    
    //figure out the overall pitch of every key currently being pressed, then get an RGB colorset for it
    int pitch = get_pitch();
    rgb(pitch);
    shift();
    
    //set every nth (dependent on petalsize, n being the base of the petal) led to black
    flowerize();
  }
  
  //if a note is currently pressed, write the valueset out to the strip
  if (noteOn == 1) {
    for (int i = 1; i<NUM_LEDS; i+= petal_size) {
      leds[i].r = r;
      leds[i].g = g;
      leds[i].b = b;
    }
  }  
  MIDI.read();
}
