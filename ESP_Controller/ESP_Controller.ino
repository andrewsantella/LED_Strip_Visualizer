#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 50
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

uint8_t brightness = 128;          // default brightness
CRGB selectedColor = CRGB::White;  // default color for visualizer/static color wheel
String lastMainMode = "";
String mainMode = "visualizer";      // visualizer or static
String vizMode = "solid";            // visualizer modes: solid, greenred
String staticEffect = "colorWheel";  // static effects: colorWheel, rainbow, chasingRainbow
uint8_t chaseSpeed = 3;              // speed for chasing rainbow

WebServer server(80);

uint8_t gamma8(uint8_t b) {
  float normalized = b / 255.0;
  float corrected = pow(normalized, 2.2);
  return (uint8_t)(corrected * 255.0 + 0.5);
}

int chasePos = 0;

void applyLEDs() {
  uint8_t correctedBrightness = gamma8(brightness);

  if (mainMode == "visualizer") {
    // Visualizer uses serial input in loop() - so just show selected color here as fallback
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = selectedColor;
      leds[i].nscale8(correctedBrightness);
    }
    FastLED.show();
  } else {
    // Static mode: apply chosen static effect
    if (staticEffect == "colorWheel") {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = selectedColor;
        leds[i].nscale8(correctedBrightness);
      }
    } else if (staticEffect == "rainbow") {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((i * 255) / NUM_LEDS, 255, correctedBrightness);
      }
    } else if (staticEffect == "chasingRainbow") {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((i * 10 + chasePos) % 255, 255, correctedBrightness);
      }
      chasePos += chaseSpeed;
    }
    FastLED.show();
  }
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html><html>
    <head>
      <title>ESP32 LED Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <style>
        body { font-family: sans-serif; padding: 10px; max-width: 400px; margin:auto; }
        label { display: inline-block; width: 140px; }
       
        .hidden { display: none; }
      </style>
      <script>
        function toggleOptions() {
  var mainMode = document.getElementById('mainMode').value;
  var staticOpts = document.getElementById('staticOptions');
  var vizOpts = document.getElementById('vizOptions');
  var staticEffect = document.getElementById('staticEffect').value;
  var speedLabel = document.getElementById('speedLabel');
  var speedSlider = document.getElementById('speed');

  var colorStaticLabel = document.querySelector("label[for='colorStatic']");
  var colorStaticInput = document.getElementById('colorStatic');

  if(mainMode === 'static') {
    staticOpts.style.display = 'block';
    vizOpts.style.display = 'none';

    // Show or hide speed slider
    if(staticEffect === 'chasingRainbow') {
      speedLabel.style.display = 'inline-block';
      speedSlider.style.display = 'inline-block';
    } else {
      speedLabel.style.display = 'none';
      speedSlider.style.display = 'none';
    }

    // Show or hide static color picker
    if (staticEffect === 'colorWheel') {
      colorStaticLabel.style.display = 'inline-block';
      colorStaticInput.style.display = 'inline-block';
    } else {
      colorStaticLabel.style.display = 'none';
      colorStaticInput.style.display = 'none';
    }

  } else {
    staticOpts.style.display = 'none';
    vizOpts.style.display = 'block';
    speedLabel.style.display = 'none';
    speedSlider.style.display = 'none';
    colorStaticLabel.style.display = 'none';
    colorStaticInput.style.display = 'none';
  }
}

      </script>
    </head>
    <body onload="toggleOptions()">
      <h2>ESP32 LED Control</h2>
      <form action="/set" method="GET">
        <label for="mainMode">Mode:</label>
        <select id="mainMode" name="mainMode" onchange="toggleOptions()">
          <option value="visualizer">Visualizer</option>
          <option value="static">Static</option>
        </select><br><br>

        <div id="vizOptions">
          <label for="color">Color:</label>
          <input type="color" id="color" name="color" value="#ffffff"><br><br>

          <label for="vizMode">Visualizer Mode:</label>
          <select id="vizMode" name="vizMode">
            <option value="solid">Solid</option>
            <option value="greenred">Green to Red</option>
          </select><br><br>
        </div>

        <div id="staticOptions" style="display:none;">
          <label for="staticEffect">Static Effect:</label>
          <select id="staticEffect" name="staticEffect" onchange="toggleOptions()">
            <option value="colorWheel">Color Wheel</option>
            <option value="rainbow">Rainbow</option>
            <option value="chasingRainbow">Chasing Rainbow</option>
          </select><br><br>

          <label for="speed" id="speedLabel" style="display:none;">Chasing Rainbow Speed:</label>
          <input type="range" id="speed" name="speed" min="1" max="20" value="3" style="display:none;"><br><br>

          <label for="colorStatic">Static Color (wheel):</label>
          <input type="color" id="colorStatic" name="colorStatic" value="#ffffff"><br><br>
        </div>

        <label for="brightness">Brightness:</label>
        <input type="range" id="brightness" name="brightness" min="0" max="255" value="128"><br><br>

        <input type="submit" value="Apply">
      </form>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("mainMode")) mainMode = server.arg("mainMode");

  if (mainMode == "visualizer") {
    if (server.hasArg("color")) {
      String hex = server.arg("color");
      long rgb = strtol(&hex[1], NULL, 16);
      selectedColor = CRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    if (server.hasArg("vizMode")) vizMode = server.arg("vizMode");
  } else if (mainMode == "static") {
    if (server.hasArg("staticEffect")) staticEffect = server.arg("staticEffect");
    if (staticEffect == "colorWheel" && server.hasArg("colorStatic")) {
      String hex = server.arg("colorStatic");
      long rgb = strtol(&hex[1], NULL, 16);
      selectedColor = CRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    if (staticEffect == "chasingRainbow" && server.hasArg("speed")) {
      chaseSpeed = constrain(server.arg("speed").toInt(), 1, 20);
    }
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
  while (Serial.available() > 0) Serial.read();  // Clear serial buffer

  WiFi.softAP("ESP32-LED", "12345678");
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  server.handleClient();

  // Handle mode transition, we need to flush the Serial buffer before resuming
  if (mainMode != lastMainMode) {
    lastMainMode = mainMode;
    while (Serial.available() > 0) {
      char t = Serial.read();
    }
  }

  if (mainMode == "visualizer") {
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
  } else {
    if (staticEffect == "chasingRainbow") {
      showChasingRainbow();
      delay(30);
    }
  }
}


void showChasingRainbow() {
  uint8_t correctedBrightness = gamma8(brightness);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((i * 10 + chasePos) % 255, 255, correctedBrightness);
  }
  FastLED.show();
  chasePos += chaseSpeed;
}
