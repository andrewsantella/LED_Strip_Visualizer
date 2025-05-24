#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 50
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

uint8_t brightness = 128;  // default brightness
CRGB selectedColor = CRGB::White;  // default color

String mainMode = "visualizer";  // visualizer or static
String staticEffect = "colorwheel";  // static effect options
String vizMode = "solid";    // visualizer mode options

WebServer server(80);

// Gamma correction function
uint8_t gamma8(uint8_t b) {
  float normalized = b / 255.0;
  float corrected = pow(normalized, 2.2);
  return (uint8_t)(corrected * 255.0 + 0.5);
}

void showStaticRainbow() {
  uint8_t correctedBrightness = gamma8(brightness);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(map(i, 0, NUM_LEDS - 1, 0, 255), 255, correctedBrightness);
  }
  FastLED.show();
}

uint8_t chasePos = 0;

void showChasingRainbow() {
  uint8_t correctedBrightness = gamma8(brightness);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((i * 10 + chasePos) % 255, 255, correctedBrightness);
  }
  FastLED.show();
  chasePos += 3;  // speed of chasing
}

void applyLEDs() {
  if (mainMode == "static") {
    if (staticEffect == "colorwheel") {
      uint8_t correctedBrightness = gamma8(brightness);
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = selectedColor;
        leds[i].nscale8(correctedBrightness);
      }
      FastLED.show();
    } else if (staticEffect == "staticRainbow") {
      showStaticRainbow();
    } 
    // chasingRainbow animated in loop()
  }
  // Visualizer LEDs updated in loop()
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html><html>
    <head><title>ESP32 LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.5, user-scalable=no">
    <script>
      function toggleOptions() {
        var mainMode = document.getElementById('mainMode').value;
        var staticOpts = document.getElementById('staticOptions');
        var vizOpts = document.getElementById('vizOptions');
        if(mainMode === 'static') {
          staticOpts.style.display = 'block';
          vizOpts.style.display = 'none';
        } else {
          staticOpts.style.display = 'none';
          vizOpts.style.display = 'block';
        }
      }
      window.onload = toggleOptions;
    </script>
    </head>
    <body style='font-family:sans-serif;'>
      <h2>ESP32 LED Control</h2>
      <form action='/set'>
        <label for='mainMode'>Mode:</label>
        <select id='mainMode' name='mainMode' onchange='toggleOptions()'>
          <option value='visualizer'>Visualizer</option>
          <option value='static'>Static</option>
        </select><br><br>

        <label for='color'>Color:</label>
        <input type='color' id='color' name='color' value='#ffffff'><br><br>

        <div id='staticOptions' style='display:none;'>
          <label for='staticEffect'>Static Effect:</label>
          <select id='staticEffect' name='staticEffect'>
            <option value='colorwheel'>Color Wheel</option>
            <option value='staticRainbow'>Static Rainbow</option>
            <option value='chasingRainbow'>Chasing Rainbow</option>
          </select><br><br>
        </div>

        <div id='vizOptions' style='display:none;'>
          <label for='vizMode'>Visualizer Mode:</label>
          <select id='vizMode' name='vizMode'>
            <option value='solid'>Solid</option>
            <option value='greenred'>Green to Red</option>
          </select><br><br>
        </div>

        <label for='brightness'>Brightness:</label>
        <input type='range' id='brightness' name='brightness' min='0' max='255' value='128'><br><br>

        <input type='submit' value='Apply'>
      </form>
    </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("mainMode")) mainMode = server.arg("mainMode");

  if (server.hasArg("color")) {
    String hex = server.arg("color");  // Format: #RRGGBB
    long rgb = strtol(&hex[1], NULL, 16);
    selectedColor = CRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
  }

  if (mainMode == "static") {
    if (server.hasArg("staticEffect")) staticEffect = server.arg("staticEffect");
  }

  if (mainMode == "visualizer") {
    if (server.hasArg("vizMode")) vizMode = server.arg("vizMode");
  }

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

  if (mainMode == "static") {
    if (staticEffect == "chasingRainbow") {
      showChasingRainbow();
      delay(50); // animation speed
    }
  } else if (mainMode == "visualizer") {
    if (Serial.available() >= NUM_LEDS) {
      for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t inputVal = Serial.read();
        uint8_t corrected = gamma8(inputVal);

        if (vizMode == "solid") {
          leds[i] = selectedColor;
          leds[i].nscale8(corrected);
        } else if (vizMode == "greenred") {
          leds[i] = CHSV(map(corrected, 0, 255, 96, 0), 255, corrected);
        }
      }
      FastLED.show();
    }
  }
}
