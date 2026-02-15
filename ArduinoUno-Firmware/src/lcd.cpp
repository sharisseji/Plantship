/**
 * Arduino Uno LCD Display - 8-Box Sensor Dashboard
 * 
 * Displays data from 2 ESP32 devices (A and B) in a 2x4 grid:
 * 
 *   ┌────────┬────────┬────────┬────────┐
 *   │ A:TEMP │ A:HUM  │ A:MOIST│ A:VOICE│
 *   ├────────┼────────┼────────┼────────┤
 *   │ B:TEMP │ B:HUM  │ B:MOIST│ B:VOICE│
 *   └────────┴────────┴────────┴────────┘
 * 
 * Protocol:
 *   S A T <float>  - Device A temperature
 *   S A H <int>    - Device A humidity
 *   S A M <int>    - Device A moisture
 *   S B T <float>  - Device B temperature
 *   S B H <int>    - Device B humidity
 *   S B M <int>    - Device B moisture
 *   V A <text>     - Device A voice (max 12 chars)
 *   V B <text>     - Device B voice (max 12 chars)
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

// Box colors - Row A (top)
#define BOX_A_TEMP_BG   0x001F  // Blue
#define BOX_A_HUMID_BG  0x07E0  // Green
#define BOX_A_MOIST_BG  0xFD20  // Orange
#define BOX_A_VOICE_BG  0x780F  // Purple

// Box colors - Row B (bottom) - slightly darker variants
#define BOX_B_TEMP_BG   0x0010  // Dark Blue
#define BOX_B_HUMID_BG  0x03E0  // Dark Green
#define BOX_B_MOIST_BG  0xFA00  // Dark Orange
#define BOX_B_VOICE_BG  0x4008  // Dark Purple

// Screen dimensions (3.5" 480x320 landscape)
#define SCREEN_W 480
#define SCREEN_H 320

// 8 boxes: 4 columns x 2 rows
#define BOX_COLS 4
#define BOX_ROWS 2
#define BOX_W (SCREEN_W / BOX_COLS)  // 120
#define BOX_H (SCREEN_H / BOX_ROWS)  // 160

// Box indices (0-7)
// Row A (top): 0-3
#define BOX_A_TEMP   0
#define BOX_A_HUMID  1
#define BOX_A_MOIST  2
#define BOX_A_VOICE  3
// Row B (bottom): 4-7
#define BOX_B_TEMP   4
#define BOX_B_HUMID  5
#define BOX_B_MOIST  6
#define BOX_B_VOICE  7

MCUFRIEND_kbv tft;

// Current values for device A
String valA_Temp = "--.-";
String valA_Humid = "--";
String valA_Moist = "---";
String valA_Voice = "READY";

// Current values for device B
String valB_Temp = "--.-";
String valB_Humid = "--";
String valB_Moist = "---";
String valB_Voice = "READY";

// Serial buffer
String serialBuffer = "";

// Get box coordinates
void getBoxCoords(int boxIndex, int* x, int* y) {
    int col = boxIndex % BOX_COLS;
    int row = boxIndex / BOX_COLS;
    *x = col * BOX_W;
    *y = row * BOX_H;
}

// Get box background color
uint16_t getBoxColor(int boxIndex) {
    switch (boxIndex) {
        case BOX_A_TEMP:   return BOX_A_TEMP_BG;
        case BOX_A_HUMID:  return BOX_A_HUMID_BG;
        case BOX_A_MOIST:  return BOX_A_MOIST_BG;
        case BOX_A_VOICE:  return BOX_A_VOICE_BG;
        case BOX_B_TEMP:   return BOX_B_TEMP_BG;
        case BOX_B_HUMID:  return BOX_B_HUMID_BG;
        case BOX_B_MOIST:  return BOX_B_MOIST_BG;
        case BOX_B_VOICE:  return BOX_B_VOICE_BG;
        default:           return BLACK;
    }
}

// Get box label
const char* getBoxLabel(int boxIndex) {
    switch (boxIndex) {
        case BOX_A_TEMP:   return "A:TEMP";
        case BOX_A_HUMID:  return "A:HUM";
        case BOX_A_MOIST:  return "A:MOIST";
        case BOX_A_VOICE:  return "A:VOICE";
        case BOX_B_TEMP:   return "B:TEMP";
        case BOX_B_HUMID:  return "B:HUM";
        case BOX_B_MOIST:  return "B:MOIST";
        case BOX_B_VOICE:  return "B:VOICE";
        default:           return "";
    }
}

// Get box value suffix
const char* getBoxSuffix(int boxIndex) {
    switch (boxIndex) {
        case BOX_A_TEMP:
        case BOX_B_TEMP:   return "C";
        case BOX_A_HUMID:
        case BOX_B_HUMID:  return "%";
        default:           return "";
    }
}

// Get pointer to value string
String* getValuePtr(int boxIndex) {
    switch (boxIndex) {
        case BOX_A_TEMP:   return &valA_Temp;
        case BOX_A_HUMID:  return &valA_Humid;
        case BOX_A_MOIST:  return &valA_Moist;
        case BOX_A_VOICE:  return &valA_Voice;
        case BOX_B_TEMP:   return &valB_Temp;
        case BOX_B_HUMID:  return &valB_Humid;
        case BOX_B_MOIST:  return &valB_Moist;
        case BOX_B_VOICE:  return &valB_Voice;
        default:           return nullptr;
    }
}

// Draw a single box with label and value
void drawBox(int boxIndex) {
    int x, y;
    getBoxCoords(boxIndex, &x, &y);
    uint16_t bgColor = getBoxColor(boxIndex);
    const char* label = getBoxLabel(boxIndex);
    const char* suffix = getBoxSuffix(boxIndex);
    String* valuePtr = getValuePtr(boxIndex);
    
    if (!valuePtr) return;
    String value = *valuePtr;
    
    // Draw background
    tft.fillRect(x + 2, y + 2, BOX_W - 4, BOX_H - 4, bgColor);
    
    // Draw border
    tft.drawRect(x, y, BOX_W, BOX_H, WHITE);
    tft.drawRect(x + 1, y + 1, BOX_W - 2, BOX_H - 2, WHITE);
    
    // Draw label at top (smaller text for 8-box layout)
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    int labelX = x + (BOX_W - strlen(label) * 6) / 2;
    tft.setCursor(labelX, y + 8);
    tft.print(label);
    
    // Draw value in center (size 3 for narrower boxes)
    tft.setTextSize(3);
    String displayVal = value + suffix;
    
    // Truncate if too long for box
    if (displayVal.length() > 6) {
        displayVal = displayVal.substring(0, 6);
    }
    
    int valX = x + (BOX_W - displayVal.length() * 18) / 2;
    int valY = y + BOX_H / 2 - 12;
    
    // Ensure text doesn't overflow left edge
    if (valX < x + 4) valX = x + 4;
    
    tft.setCursor(valX, valY);
    tft.print(displayVal);
}

// Draw all 8 boxes
void drawAllBoxes() {
    for (int i = 0; i < 8; i++) {
        drawBox(i);
    }
}

// Parse and handle serial command
void handleCommand(const String& line) {
    if (line.length() < 4) return;
    
    // Sensor commands: "S A T 23.7" or "S B H 41"
    if (line.startsWith("S ")) {
        char device = line.charAt(2);
        char sensor = line.charAt(4);
        String value = line.substring(6);
        value.trim();
        
        int boxIndex = -1;
        
        if (device == 'A') {
            if (sensor == 'T') { valA_Temp = value; boxIndex = BOX_A_TEMP; }
            else if (sensor == 'H') { valA_Humid = value; boxIndex = BOX_A_HUMID; }
            else if (sensor == 'M') { valA_Moist = value; boxIndex = BOX_A_MOIST; }
        }
        else if (device == 'B') {
            if (sensor == 'T') { valB_Temp = value; boxIndex = BOX_B_TEMP; }
            else if (sensor == 'H') { valB_Humid = value; boxIndex = BOX_B_HUMID; }
            else if (sensor == 'M') { valB_Moist = value; boxIndex = BOX_B_MOIST; }
        }
        
        if (boxIndex >= 0) {
            drawBox(boxIndex);
            Serial.print("OK ");
            Serial.print(device);
            Serial.print(" ");
            Serial.println(sensor);
        } else {
            Serial.print("ERR Bad sensor: ");
            Serial.println(line);
        }
    }
    // Voice commands: "V A HELLO" or "V B WORLD"
    else if (line.startsWith("V ")) {
        char device = line.charAt(2);
        String text = line.substring(4);
        text.trim();
        
        // Truncate voice text for display
        if (text.length() > 6) {
            text = text.substring(0, 6);
        }
        
        int boxIndex = -1;
        
        if (device == 'A') {
            valA_Voice = text;
            boxIndex = BOX_A_VOICE;
        }
        else if (device == 'B') {
            valB_Voice = text;
            boxIndex = BOX_B_VOICE;
        }
        
        if (boxIndex >= 0) {
            drawBox(boxIndex);
            Serial.print("OK V ");
            Serial.println(device);
        } else {
            Serial.print("ERR Bad device: ");
            Serial.println(line);
        }
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
    
    Serial.println("LCD 8-Box Ready");
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