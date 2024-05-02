#include "Adafruit_seesaw.h"
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

bool ssEnable;
Adafruit_seesaw ss;

WebServer server(80);

int batteryPin = 1;

void configureSeeSaw() {
  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    return;
  }

  ssEnable = true;

  Serial.print("seesaw started! version: ");
  Serial.println(ss.getVersion(), HEX);
}

void configureWiFi() {
  // We start by connecting to a WiFi network
    WiFi.begin("ssid", "password");

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi...");

    auto LEVEL = LOW;
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
        digitalWrite(LED_BUILTIN, LEVEL);
        if (LEVEL == HIGH) {
          LEVEL = LOW;
        } else {
          LEVEL = HIGH;
        }
    }

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/metrics", metrics);
    server.on("/", metrics);

    server.begin();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // battery
  pinMode(batteryPin, INPUT);
  
  // sensor
  configureSeeSaw();

  // wifi
  delay(100);
  configureWiFi();
}

void metrics() {
  float tempC = 0;
  uint16_t capread = 0;
  
  if (ssEnable) {
    tempC = ss.getTemp();
    capread = ss.touchRead(0);
  }

  String msg;

  msg += "# HELP soil_temperature The temperature of the soil\n";
  msg += "# TYPE soil_temperature gauge\n";
  msg += "soil_temperature " + String(tempC) + "\n";
  
  msg += "# HELP soil_capacitance The capacitance of the soil\n";
  msg += "# TYPE soil_capacitance gauge\n";
  msg += "soil_capacitance " + String(capread) + "\n";

  msg += "# HELP battery_voltage The voltage of the battery\n";
  msg += "# TYPE battery_voltage gauge\n";
  msg += "battery_voltage " + String(readVCC()) + "\n";

  server.send(200, "text/plain", msg);
}

float readVCC() {
  uint8_t numSamples = 100;
  uint32_t batteryRaw = 0;
  
  for(uint8_t i = 0; i < numSamples; i++) {
      batteryRaw += analogRead(batteryPin);
  }

  float voltage = 3.3 / 4095 * batteryRaw / numSamples;
  voltage *= 1.688;

  return voltage;
}

void loop() {
  server.handleClient();
}
