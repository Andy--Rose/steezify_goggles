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

#define PIN_LEDS 10
#define LEDS 40
#define BRIGHTNESS 64

//
//int design[13][4] = {
//  {0, 1, 2, 0},
//  {0, 3, 4, 5},
//  {0, 6, 7, 8},
//  {9, 10, 11, 12},
//  {0, 13, 14, 15},
//  {0, 0, 16, 17},
//  {0, 18, 19, 20},
//  {21, 22, 23, 24},
//  {25, 26, 27, 28},
//  {29, 30, 31, 32},
//  {0, 33, 34, 35},
//  {0, 36, 37, 38},
//  {0, 39, 40, 0}
//};

CRGB leds[LEDS];
GogglePattern goggles = GogglePattern(LEDS, leds, &GogglesComplete);

const int buttonPin = 6;

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");
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
  goggles.Update();
}

void test() {
  Serial.println("Entering test mode");
//  Test Patterns
  goggles.LockPattern();

//  Test Clap
//  goggles.TestClap();
  goggles.TestRainbow();
}


void ButtonHandler() {
  if (digitalRead(buttonPin) == HIGH) {
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

void GogglesComplete()
{
  if (goggles.GetPattern() == COLOR_WIPE) {
    goggles.Reverse();
  }
}

