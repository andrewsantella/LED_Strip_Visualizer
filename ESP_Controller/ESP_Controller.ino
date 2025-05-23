#include <FastLED.h>

#define NUM_LEDS 50      // Change to your number of LEDs
#define DATA_PIN 5       // GPIO pin connected to LED strip

CRGB leds[NUM_LEDS];

// Gamma correction lookup table for 8-bit input (0-255)
// You can also calculate this on the fly if you want
uint8_t gamma8(uint8_t brightness) {
  // Gamma 2.2 correction
  float normalized = brightness / 255.0;
  float corrected = pow(normalized, 2.2);
  return (uint8_t)(corrected * 255.0 + 0.5);
}

void setup() {
  Serial.begin(460800);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  if (Serial.available() >= NUM_LEDS) {
    for (int i = 0; i < NUM_LEDS; i++) {
      uint8_t inputVal = Serial.read();
      uint8_t correctedBrightness = gamma8(inputVal);

      // Set LED to white with corrected brightness
      leds[i] = CRGB(correctedBrightness, correctedBrightness, correctedBrightness);
    }
    FastLED.show();
  }
}
