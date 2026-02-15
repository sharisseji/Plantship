/**
 * Arduino Uno LCD Display - 4-Box Sensor Dashboard
 * 
 * Receives serial commands from PC server and displays:
 * - Temperature (top-left)
 * - Humidity (top-right)  
 * - Moisture (bottom-left)
 * - Voice text (bottom-right)
 * 
 * Protocol:
 *   S T <float>  - Set temperature
 *   S H <int>    - Set humidity
 *   S M <int>    - Set moisture
 *   V <text>     - Set voice text (max 20 chars)
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <Arduino.h>

// Colors
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define ORANGE  0xFD20
#define GRAY    0x8410
#define DARKGRAY 0x4208

// Box colors
#define BOX_TEMP_BG    0x001F  // Blue
#define BOX_HUMID_BG   0x07E0  // Green
#define BOX_MOIST_BG   0xFD20  // Orange
#define BOX_VOICE_BG   0x780F  // Purple

// Screen dimensions (3.5" 480x320 landscape)
#define SCREEN_W 480
#define SCREEN_H 320
#define BOX_W (SCREEN_W / 2)
#define BOX_H (SCREEN_H / 2)

// Box indices
#define BOX_TEMP   0
#define BOX_HUMID  1
#define BOX_MOIST  2
#define BOX_VOICE  3

MCUFRIEND_kbv tft;

// Current values
String currentTemp = "--.-";
String currentHumid = "--";
String currentMoist = "---";
String currentVoice = "READY";

// Serial buffer
String serialBuffer = "";

// Get box coordinates
void getBoxCoords(int boxIndex, int* x, int* y) {
    switch (boxIndex) {
        case BOX_TEMP:   *x = 0;      *y = 0;      break;
        case BOX_HUMID:  *x = BOX_W;  *y = 0;      break;
        case BOX_MOIST:  *x = 0;      *y = BOX_H;  break;
        case BOX_VOICE:  *x = BOX_W;  *y = BOX_H;  break;
    }
}

// Get box background color
uint16_t getBoxColor(int boxIndex) {
    switch (boxIndex) {
        case BOX_TEMP:   return BOX_TEMP_BG;
        case BOX_HUMID:  return BOX_HUMID_BG;
        case BOX_MOIST:  return BOX_MOIST_BG;
        case BOX_VOICE:  return BOX_VOICE_BG;
        default:         return BLACK;
    }
}

// Get box label
const char* getBoxLabel(int boxIndex) {
    switch (boxIndex) {
        case BOX_TEMP:   return "TEMP";
        case BOX_HUMID:  return "HUMID";
        case BOX_MOIST:  return "MOIST";
        case BOX_VOICE:  return "VOICE";
        default:         return "";
    }
}

// Get box value suffix
const char* getBoxSuffix(int boxIndex) {
    switch (boxIndex) {
        case BOX_TEMP:   return "C";
        case BOX_HUMID:  return "%";
        case BOX_MOIST:  return "";
        case BOX_VOICE:  return "";
        default:         return "";
    }
}

// Draw a single box with label and value
void drawBox(int boxIndex, const String& value) {
    int x, y;
    getBoxCoords(boxIndex, &x, &y);
    uint16_t bgColor = getBoxColor(boxIndex);
    const char* label = getBoxLabel(boxIndex);
    const char* suffix = getBoxSuffix(boxIndex);
    
    // Draw background
    tft.fillRect(x + 2, y + 2, BOX_W - 4, BOX_H - 4, bgColor);
    
    // Draw border
    tft.drawRect(x, y, BOX_W, BOX_H, WHITE);
    tft.drawRect(x + 1, y + 1, BOX_W - 2, BOX_H - 2, WHITE);
    
    // Draw label at top
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    int labelX = x + (BOX_W - strlen(label) * 12) / 2;
    tft.setCursor(labelX, y + 15);
    tft.print(label);
    
    // Draw value in center
    tft.setTextSize(4);
    String displayVal = value + suffix;
    int valX = x + (BOX_W - displayVal.length() * 24) / 2;
    int valY = y + BOX_H / 2 - 16;
    tft.setCursor(valX, valY);
    tft.print(displayVal);
}

// Draw all boxes
void drawAllBoxes() {
    drawBox(BOX_TEMP, currentTemp);
    drawBox(BOX_HUMID, currentHumid);
    drawBox(BOX_MOIST, currentMoist);
    drawBox(BOX_VOICE, currentVoice);
}

// Parse and handle serial command
void handleCommand(const String& line) {
    if (line.length() < 2) return;
    
    // Temperature: "S T 23.7"
    if (line.startsWith("S T ")) {
        currentTemp = line.substring(4);
        currentTemp.trim();
        drawBox(BOX_TEMP, currentTemp);
        Serial.println("OK TEMP");
    }
    // Humidity: "S H 41"
    else if (line.startsWith("S H ")) {
        currentHumid = line.substring(4);
        currentHumid.trim();
        drawBox(BOX_HUMID, currentHumid);
        Serial.println("OK HUMID");
    }
    // Moisture: "S M 78"
    else if (line.startsWith("S M ")) {
        currentMoist = line.substring(4);
        currentMoist.trim();
        drawBox(BOX_MOIST, currentMoist);
        Serial.println("OK MOIST");
    }
    // Voice: "V LIGHTS ON"
    else if (line.startsWith("V ")) {
        currentVoice = line.substring(2);
        currentVoice.trim();
        if (currentVoice.length() > 10) {
            // For long text, use smaller font handled in drawBox
            currentVoice = currentVoice.substring(0, 10);
        }
        drawBox(BOX_VOICE, currentVoice);
        Serial.println("OK VOICE");
    }
    else {
        Serial.print("ERR Unknown: ");
        Serial.println(line);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    tft.reset();
    uint16_t id = tft.readID();
    if (id == 0x00D3) id = 0x9481;  // Common fix
    tft.begin(id);
    tft.setRotation(1);  // Landscape
    tft.fillScreen(BLACK);
    
    // Draw initial UI
    drawAllBoxes();
    
    Serial.println("LCD Ready");
}

void loop() {
    // Read serial data
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n') {
            // Process complete line
            serialBuffer.trim();
            if (serialBuffer.length() > 0) {
                handleCommand(serialBuffer);
            }
            serialBuffer = "";
        }
        else if (c != '\r') {
            serialBuffer += c;
            
            // Prevent buffer overflow
            if (serialBuffer.length() > 64) {
                serialBuffer = "";
            }
        }
    }
}