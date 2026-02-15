/**
 * ESP32 Sensor Hub - Sends sensor data to PC server via HTTP
 * 
 * Sensors:
 * - DHT11 (temperature + humidity) on GPIO 16
 * - Moisture sensor on GPIO 15
 * 
 * Sends JSON to: http://<SERVER_IP>:5000/sensor
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHTesp.h>

// ============== CONFIGURATION ==============
// WiFi credentials - UPDATE THESE
const char* WIFI_SSID = "Sharisse";
const char* WIFI_PASSWORD = "panictime";

// Server URL - UPDATE with your PC's IP address
const char* SERVER_URL = "http://172.20.10.3:5000/sensor";

// Sensor pins
const int DHT_PIN = 16;
const int MOISTURE_PIN = 15;

// Timing
const unsigned long SEND_INTERVAL_MS = 2000;  // Send every 2 seconds
const int WIFI_RETRY_DELAY_MS = 500;
const int MAX_WIFI_RETRIES = 20;
const int HTTP_TIMEOUT_MS = 5000;
// ===========================================

DHTesp dht;
unsigned long lastSendTime = 0;

void connectWiFi() {
    Serial.print("[WiFi] Connecting to ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < MAX_WIFI_RETRIES) {
        delay(WIFI_RETRY_DELAY_MS);
        Serial.print(".");
        retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("[WiFi] Connection failed - will retry later");
    }
}

bool sendSensorData(float temp, float humidity, int moisture) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi not connected, skipping send");
        return false;
    }
    
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(HTTP_TIMEOUT_MS);
    
    // Build JSON payload
    String json = "{";
    json += "\"temp\":" + String(temp, 1) + ",";
    json += "\"humidity\":" + String((int)humidity) + ",";
    json += "\"moisture\":" + String(moisture);
    json += "}";
    
    Serial.print("[HTTP] POST ");
    Serial.print(SERVER_URL);
    Serial.print(" -> ");
    Serial.println(json);
    
    int httpCode = http.POST(json);
    
    if (httpCode > 0) {
        Serial.print("[HTTP] Response: ");
        Serial.println(httpCode);
        if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            Serial.println(response);
        }
        http.end();
        return true;
    } else {
        Serial.print("[HTTP] Error: ");
        Serial.println(http.errorToString(httpCode));
        http.end();
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("================================");
    Serial.println("ESP32 Sensor Hub Starting...");
    Serial.println("================================");
    
    // Initialize DHT sensor
    dht.setup(DHT_PIN, DHTesp::DHT11);
    Serial.println("[Sensor] DHT11 initialized on GPIO " + String(DHT_PIN));
    Serial.println("[Sensor] Moisture on GPIO " + String(MOISTURE_PIN));
    
    // Connect to WiFi
    connectWiFi();
    
    Serial.println("[Ready] Sending data to " + String(SERVER_URL));
    Serial.println("================================");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Reconnect WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Reconnecting...");
        connectWiFi();
    }
    
    // Send sensor data at interval
    if (currentTime - lastSendTime >= SEND_INTERVAL_MS) {
        lastSendTime = currentTime;
        
        TempAndHumidity data = dht.getTempAndHumidity();
        int moisture = analogRead(MOISTURE_PIN);
        
        if (dht.getStatus() != DHTesp::ERROR_NONE) {
            Serial.print("[Sensor] Error: ");
            Serial.println(dht.getStatusString());
        } else {
            Serial.println("--------------------------------");
            Serial.print("Temp: ");
            Serial.print(data.temperature, 1);
            Serial.print("C | Humidity: ");
            Serial.print(data.humidity, 0);
            Serial.print("% | Moisture: ");
            Serial.println(moisture);
            
            sendSensorData(data.temperature, data.humidity, moisture);
        }
    }
    
    // Small delay to prevent tight loop
    delay(100);
}