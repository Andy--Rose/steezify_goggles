#include <Adafruit_NeoPixel.h>
#include "GogglePattern.h"

#define PIN_LEDS 10
#define LEDS 40

int design[13][4] = {
  {0, 1, 2, 0},
  {0, 3, 4, 5},
  {0, 6, 7, 8},
  {9, 10, 11, 12},
  {0, 13, 14, 15},
  {0, 0, 16, 17},
  {0, 18, 19, 20},
  {21, 22, 23, 24},
  {25, 26, 27, 28},
  {29, 30, 31, 32},
  {0, 33, 34, 35},
  {0, 36, 37, 38},
  {0, 39, 40, 0}
};

GogglePattern goggles = GogglePattern(LEDS, PIN_LEDS, (int*)design, NEO_GRB + NEO_KHZ800, &GogglesComplete);

const int buttonPin = 3;

void setup() {
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  goggles.begin();

  attachInterrupt(digitalPinToInterrupt(buttonPin), ButtonHandler, CHANGE);
}

void loop() {
  goggles.Update();
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
  if (goggles.GetPattern() == "COLOR_WIPE") {
    goggles.Reverse();
  }
}
