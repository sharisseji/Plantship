#include <Arduino.h>
#include <DHTesp.h>

const int DHT_PIN = 16;
const int MOISTURE_PIN = 15;
DHTesp dht;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    dht.setup(DHT_PIN, DHTesp::DHT11);
    Serial.println("DHT11 sensor initialized");
}

void loop()
{
    TempAndHumidity data = dht.getTempAndHumidity();
    
    if (dht.getStatus() != DHTesp::ERROR_NONE) {
        Serial.print("Sensor error: ");
        Serial.println(dht.getStatusString());
    } else {
        Serial.print("Humidity = ");
        Serial.print(data.humidity, 1);
        Serial.println('%');
        Serial.print("Temperature = ");
        Serial.print(data.temperature, 1);
        Serial.println('C');
        Serial.println("-- OK");
        Serial.println("Moisture Sensor Value:");
        Serial.println(analogRead(MOISTURE_PIN));
        delay(1000); 
    }
    
    delay(2000); // DHT11 needs at least 1-2 seconds between readings
}