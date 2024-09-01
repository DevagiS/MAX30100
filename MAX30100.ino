#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000

const char* ssid = "Deki.";
const char* password = "deki2826.";

ESP8266WebServer server(80);
PulseOximeter pox;
float BPM, SpO2;
uint32_t tsLastReport = 0;

void onBeatDetected() {
    Serial.println("Beat Detected!");
}

void handleRoot() {
    String html = "<!DOCTYPE html>"
                  "<html lang=\"en\">"
                  "<head>"
                  "<meta charset=\"UTF-8\">"
                  "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
                  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                  "<title>Pulse Oximeter Dashboard</title>"
                  "<style>"
                  "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f9; color: #333; }"
                  ".container { text-align: center; margin-top: 50px; }"
                  "h1 { font-size: 2.5em; color: #4CAF50; }"
                  ".data { font-size: 2em; margin: 20px; padding: 15px; border-radius: 8px; background-color: #fff; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); display: inline-block; }"
                  "#bpm { color: #2196F3; }"
                  "#spo2 { color: #FF5722; }"
                  "footer { margin-top: 30px; font-size: 0.9em; color: #888; }"
                  "footer a { color: #2196F3; text-decoration: none; }"
                  "footer a:hover { text-decoration: underline; }"
                  "</style>"
                  "</head>"
                  "<body>"
                  "<div class=\"container\">"
                  "<h1>Pulse Oximeter Dashboard</h1>"
                  "<div class=\"data\">"
                  "Heart Rate (BPM): <span id=\"bpm\">0</span>"
                  "</div>"
                  "<div class=\"data\">"
                  "Oxygen Saturation (SpO2): <span id=\"spo2\">0</span>%"
                  "</div>"
                  "<footer>"
                  "<p>Data updated every second. <a href=\"https://github.com/DevagiS\">View on GitHub</a></p>"
                  "</footer>"
                  "<script>"
                  "async function fetchData() {"
                  "try {"
                  "const response = await fetch('/data');"
                  "const data = await response.json();"
                  "document.getElementById('bpm').textContent = data.BPM.toFixed(1);"
                  "document.getElementById('spo2').textContent = data.SpO2.toFixed(1);"
                  "} catch (error) {"
                  "console.error('Error fetching data:', error);"
                  "}"
                  "}"
                  "setInterval(fetchData, 1000);"
                  "</script>"
                  "</body>"
                  "</html>";
    
    server.send(200, "text/html", html);
}

void handleData() {
    String jsonResponse = "{\"BPM\":" + String(BPM) + ",\"SpO2\":" + String(SpO2) + "}";
    server.send(200, "application/json", jsonResponse);
}

void setup() {
    Serial.begin(115200);
    Wire.begin(4, 5);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Web server started");

    if (!pox.begin()) {
        Serial.println("FAILED");
        while (true);
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop() {
    pox.update();
    server.handleClient();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate: ");
        Serial.print(BPM);
        Serial.print(" bpm / SpO2: ");
        Serial.print(SpO2);
        Serial.println(" %");
        tsLastReport = millis();
    }
}