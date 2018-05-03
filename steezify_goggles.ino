/* README
 *  - Use Arduino Uno to view Serial messages for debugging
 *  - Use the board manager to ad adafruit if they don't exist
 *  - If Adafruit not available in Board Manager, add URL to preferences on additional board locations
 *  - Arduino is C++. Easier to search for C++ solutions
 * Trinket: 
 *   Board - Trinket 5V/16Hz (USB)
 *   Programmer - USBTinyISP
 * 
 * Links:
 * https://github.com/FastLED/FastLED/wiki/Pixel-reference
*/

//#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#include "GogglePattern.h"
#include <IRremote.h>

#define PIN_LEDS 10
#define PIN_REMOTE 6
#define PIN_BUTTON 2
#define LEDS 40
#define BRIGHTNESS 64

CRGB leds[LEDS];
GogglePattern goggles = GogglePattern(LEDS, leds);

IRrecv irrecv(PIN_REMOTE);
decode_results results;

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");

  irrecv.enableIRIn(); // Start the receiver
//  pinMode(buttonPin, INPUT);
//  digitalWrite(buttonPin, HIGH);
//  goggles.begin();
  
  FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, LEDS);
  FastLED.show();
  goggles.Init(NORMAL, 255);

  test();
//  attachInterrupt(digitalPinToInterrupt(buttonPin), ButtonHandler, CHANGE);
}

void loop() {
  if (irrecv.decode(&results))
  {
   Serial.println(results.value, HEX);
   remoteCommand();
   irrecv.resume(); // Receive the next value
  }
  goggles.Update();
}

void remoteCommand() {
  if (results.value == 0xD || results.value == 0x80D) {
    Serial.println("Rotate Pattern");
    goggles.UnlockPattern();
  }
  else if (results.value == 1 || results.value == 0x801) {
    Serial.println("Rainbow");
    goggles.LockPattern();
    goggles.SetRainbow();
  }
  else if (results.value == 2 || results.value == 0x802) {
    Serial.println("Color Wipe");
    goggles.LockPattern();
    goggles.SetColorWipe();
  }
  else if (results.value == 3 || results.value == 0x803) {
    Serial.println("Theater Chase");
    goggles.LockPattern();
    goggles.SetTheaterChase();
  }
  else if (results.value == 4 || results.value == 0x804) {
    Serial.println("Loopy");
    goggles.LockPattern();
    goggles.SetLoopy();
  }
}

void test() {
  Serial.println("Entering test mode");
  // Test Patterns
//  goggles.LockPattern();

  // Test methods
//  goggles.SetRainbow();
//  goggles.SetColorWipe();
//  goggles.SetTheaterChase();
//  goggles.SetLoopy();
//  goggles.SetWave();
//    goggles.SetClap();
}


void ButtonHandler() {
  if (digitalRead(PIN_BUTTON) == HIGH) {
//    Serial.begin(9600);
//    Serial.println();
//    Serial.println("Button Pressed");
//    Serial.println("Changed pattern from " + (String)goggles.PatternIndex);
    goggles.NextPattern();
    delay(2000);
//    Serial.println("Changed pattern to " + (String)goggles.PatternIndex + " which correlates to " + goggles.GetPattern());
//    Serial.end();
  }
}

