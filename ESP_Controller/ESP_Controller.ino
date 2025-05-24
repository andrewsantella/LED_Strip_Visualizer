#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 150
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

uint8_t brightness = 255;
CRGB selectedColor = CRGB::White;
String lastMainMode = "";
String mainMode = "visualizer";
String vizMode = "solid";
String staticEffect = "colorWheel";
uint8_t chaseSpeed = 3;
uint8_t rainbowWidth = 10;  // New: rainbow width control

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
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = selectedColor;
      leds[i].nscale8(correctedBrightness);
    }
    FastLED.show();
  } else {
    if (staticEffect == "colorWheel") {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = selectedColor;
        leds[i].nscale8(correctedBrightness);
      }
    } else if (staticEffect == "rainbow") {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((i * rainbowWidth) % 255, 255, correctedBrightness);
      }
    } else if (staticEffect == "chasingRainbow") {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((i * rainbowWidth + chasePos) % 255, 255, correctedBrightness);
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
          var widthLabel = document.getElementById('widthLabel');
          var widthSlider = document.getElementById('rainbowWidth');
          var colorStaticLabel = document.querySelector("label[for='colorStatic']");
          var colorStaticInput = document.getElementById('colorStatic');

          if(mainMode === 'static') {
            staticOpts.style.display = 'block';
            vizOpts.style.display = 'none';

            if(staticEffect === 'chasingRainbow') {
              speedLabel.style.display = 'inline-block';
              speedSlider.style.display = 'inline-block';
            } else {
              speedLabel.style.display = 'none';
              speedSlider.style.display = 'none';
            }

            if (staticEffect === 'rainbow' || staticEffect === 'chasingRainbow') {
              widthLabel.style.display = 'inline-block';
              widthSlider.style.display = 'inline-block';
            } else {
              widthLabel.style.display = 'none';
              widthSlider.style.display = 'none';
            }

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
            widthLabel.style.display = 'none';
            widthSlider.style.display = 'none';
            colorStaticLabel.style.display = 'none';
            colorStaticInput.style.display = 'none';
          }
        }
        function updateInvertedSlider() {
    const slider = document.getElementById('rainbowWidth');
    const hiddenInput = document.getElementById('rainbowWidthHidden');
    hiddenInput.value = 51 - parseInt(slider.value); // Reverse the meaning
  }

  document.addEventListener("DOMContentLoaded", function() {
    const slider = document.getElementById('rainbowWidth');
    slider.addEventListener("input", updateInvertedSlider);
    updateInvertedSlider(); // run once on load
  });
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
            <option value="rainbow">Static Rainbow</option>
            <option value="chasingRainbow">Chasing Rainbow</option>
          </select><br><br>

          <label for="speed" id="speedLabel" style="display:none;">Chasing Rainbow Speed:</label>
          <input type="range" id="speed" name="speed" min="1" max="20" value="3" style="display:none;"><br><br>

          <label for="rainbowWidth" id="widthLabel" style="display:none;">Rainbow Width:</label>
          <input type="range" id="rainbowWidth" min="5" max="50" step="5" value="30" style="display:none;" oninput="this.setAttribute('data-value', 51 - this.value);">

<input type="hidden" id="rainbowWidthHidden" name="rainbowWidth" value="10">


          <label for="colorStatic">Static Color:</label>
          <input type="color" id="colorStatic" name="colorStatic" value="#ffffff"><br><br>
        </div>

        <label for="brightness">Brightness:</label>
        <input type="range" id="brightness" name="brightness" min="75" max="255" value="255"><br><br>

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

    if ((staticEffect == "rainbow" || staticEffect == "chasingRainbow") && server.hasArg("rainbowWidth")) {
      rainbowWidth = constrain(server.arg("rainbowWidth").toInt(), 1, 50);
    }
  }

  if (server.hasArg("brightness")) brightness = server.arg("brightness").toInt();

  applyLEDs();

  server.sendHeader("Location", "/");
  server.send(303);
}


void showChasingRainbow() {
  uint8_t correctedBrightness = gamma8(brightness);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((i * rainbowWidth + chasePos) % 255, 255, correctedBrightness);
  }
  FastLED.show();
  chasePos += chaseSpeed;
}

void setup() {
  delay(1000);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  delay(250);
  Serial.begin(921600);
  delay(250);
  while (Serial.available() > 0) Serial.read();

  WiFi.softAP("ESP32-LED", "12345678");
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  server.handleClient();

  if (mainMode != lastMainMode) {
    lastMainMode = mainMode;
    while (Serial.available() > 0) {
      char t = Serial.read();
    }
  }

  if (mainMode == "visualizer") {
    const int numBytes = NUM_LEDS / 2;
    if (Serial.available() >= numBytes) {
      uint8_t brightnessCorrected = gamma8(brightness);

      for (int i = 0; i < NUM_LEDS; i += 2) {
        uint8_t packedByte = Serial.read();
        uint8_t val1 = packedByte >> 4;    // high nibble
        uint8_t val2 = packedByte & 0x0F;  // low nibble

        uint8_t corrected1 = gamma8(val1 * 17);  // scale 0–15 to 0–255
        uint8_t corrected2 = gamma8(val2 * 17);

        uint8_t final1 = (uint16_t)corrected1 * brightnessCorrected / 255;
        uint8_t final2 = (uint16_t)corrected2 * brightnessCorrected / 255;

        if (vizMode == "solid") {
          leds[i] = selectedColor;
          leds[i].nscale8(final1);
          if (i + 1 < NUM_LEDS) {
            leds[i + 1] = selectedColor;
            leds[i + 1].nscale8(final2);
          }
        } else if (vizMode == "greenred") {
          leds[i] = CHSV(map(final1, 0, 255, 96, 0), 255, final1);
          if (i + 1 < NUM_LEDS) {
            leds[i + 1] = CHSV(map(final2, 0, 255, 96, 0), 255, final2);
          }
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
