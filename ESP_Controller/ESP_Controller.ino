#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 50
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

uint8_t brightness = 128;
String colorMode = "white";  // Options: white, red, orange, blue
String vizMode = "solid";    // Options: solid, greenred

WebServer server(80);

// Gamma correction function
uint8_t gamma8(uint8_t b) {
  float normalized = b / 255.0;
  float corrected = pow(normalized, 2.2);
  return (uint8_t)(corrected * 255.0 + 0.5);
}

CRGB getColorFromMode(String mode) {
  if (mode == "red") return CRGB::Red;
  if (mode == "orange") return CRGB::Orange;
  if (mode == "blue") return CRGB::Blue;
  return CRGB::White;
}

void applyLEDs() {
  uint8_t correctedBrightness = gamma8(brightness);
  CRGB color = getColorFromMode(colorMode);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    leds[i].nscale8(correctedBrightness);
  }
  FastLED.show();
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html><html>
    <head><title>ESP32 LED Control</title></head>
    <body style='font-family:sans-serif;'>
      <h2>ESP32 LED Control</h2>
      <form action='/set'>
        <label>Color:</label>
        <select name='color'>
          <option value='white'>White</option>
          <option value='red'>Red</option>
          <option value='orange'>Orange</option>
          <option value='blue'>Blue</option>
        </select><br><br>
        <label>Visualizer Mode:</label>
        <select name='mode'>
          <option value='solid'>Solid</option>
          <option value='greenred'>Green to Red</option>
        </select><br><br>
        <label for='brightness'>Brightness:</label>
        <input type='range' id='brightness' name='brightness' min='0' max='255' value='128'><br><br>
        <input type='submit' value='Apply'>
      </form>
    </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("color")) colorMode = server.arg("color");
  if (server.hasArg("mode")) vizMode = server.arg("mode");
  if (server.hasArg("brightness")) brightness = server.arg("brightness").toInt();

  applyLEDs();

  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  delay(1000);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  delay(250);
  Serial.begin(460800);
  delay(250);
  while (Serial.available() > 0) Serial.read(); // Clear serial buffer

  WiFi.softAP("ESP32-LED", "12345678");
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  server.handleClient();

  if (Serial.available() >= NUM_LEDS) {
    for (int i = 0; i < NUM_LEDS; i++) {
      uint8_t inputVal = Serial.read();
      uint8_t corrected = gamma8(inputVal);

      if (vizMode == "solid") {
        CRGB color = getColorFromMode(colorMode);
        leds[i] = color;
        leds[i].nscale8(corrected);
      } else if (vizMode == "greenred") {
        leds[i] = CHSV(map(corrected, 0, 255, 96, 0), 255, corrected); // Green to red
      }
    }
    FastLED.show();
  }
}
