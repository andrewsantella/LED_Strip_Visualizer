#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 50
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

// Default LED values
uint8_t brightness = 128;
CRGB selectedColor = CRGB::White;


WebServer server(80);

// Gamma correction function
uint8_t gamma8(uint8_t brightness) {
  float normalized = brightness / 255.0;
  float corrected = pow(normalized, 2.2);
  return (uint8_t)(corrected * 255.0 + 0.5);
}

void applyLEDs() {
  uint8_t correctedBrightness = gamma8(brightness);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = selectedColor;
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
        <label for='color'>Color:</label>
        <input type='color' id='color' name='color' value='#ffffff'><br><br>
        <label for='brightness'>Brightness:</label>
        <input type='range' id='brightness' name='brightness' min='0' max='255' value='128'><br><br>
        <input type='submit' value='Apply'>
      </form>
    </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("color") && server.hasArg("brightness")) {
    String hex = server.arg("color");  // Format: #RRGGBB
    long rgb = strtol(&hex[1], NULL, 16);
    selectedColor = CRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    brightness = server.arg("brightness").toInt();
    applyLEDs();
  }
  server.sendHeader("Location", "/");
  server.send(303);  // Redirect to root
}


void setup() {
  delay(1000);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  delay(250);
  Serial.begin(460800);
  delay(250);



  // Start Wi-Fi AP
  WiFi.softAP("ESP32-LED", "12345678");
  IPAddress IP = WiFi.softAPIP();

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  delay(250);


  while (Serial.available() > 0) {
    Serial.read();  // Discard anything in the buffer
  }
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

  server.handleClient();
}
