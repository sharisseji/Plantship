// /**
//  * Arduino Uno LCD Display - 4-Box Sensor Dashboard
//  *
//  * Receives serial commands from PC server and displays:
//  * - Temperature (top-left)
//  * - Humidity (top-right)
//  * - Moisture (bottom-left)
//  * - Voice text (bottom-right)
//  *
//  * Protocol:
//  *   S T <float>  - Set temperature
//  *   S H <int>    - Set humidity
//  *   S M <int>    - Set moisture
//  *   V <text>     - Set voice text (max 20 chars)
//  */

// #include <Adafruit_GFX.h>
// #include <MCUFRIEND_kbv.h>
// #include <Arduino.h>

// // Colors
// #define BLACK   0x0000
// #define WHITE   0xFFFF
// #define RED     0xF800
// #define GREEN   0x07E0
// #define BLUE    0x001F
// #define CYAN    0x07FF
// #define YELLOW  0xFFE0
// #define ORANGE  0xFD20
// #define GRAY    0x8410
// #define DARKGRAY 0x4208

// // Box colors
// #define BOX_TEMP_BG    0x001F  // Blue
// #define BOX_HUMID_BG   0x07E0  // Green
// #define BOX_MOIST_BG   0xFD20  // Orange
// #define BOX_VOICE_BG   0x780F  // Purple

// // Screen dimensions (3.5" 480x320 landscape)
// #define SCREEN_W 480
// #define SCREEN_H 320
// #define BOX_W (SCREEN_W / 2)
// #define BOX_H (SCREEN_H / 2)

// // Box indices
// #define BOX_TEMP   0
// #define BOX_HUMID  1
// #define BOX_MOIST  2
// #define BOX_VOICE  3

// MCUFRIEND_kbv tft;

// // Current values
// String currentTemp = "--.-";
// String currentHumid = "--";
// String currentMoist = "---";
// String currentVoice = "READY";

// // Serial buffer
// String serialBuffer = "";

// // Get box coordinates
// void getBoxCoords(int boxIndex, int* x, int* y) {
//     switch (boxIndex) {
//         case BOX_TEMP:   *x = 0;      *y = 0;      break;
//         case BOX_HUMID:  *x = BOX_W;  *y = 0;      break;
//         case BOX_MOIST:  *x = 0;      *y = BOX_H;  break;
//         case BOX_VOICE:  *x = BOX_W;  *y = BOX_H;  break;
//     }
// }

// // Get box background color
// uint16_t getBoxColor(int boxIndex) {
//     switch (boxIndex) {
//         case BOX_TEMP:   return BOX_TEMP_BG;
//         case BOX_HUMID:  return BOX_HUMID_BG;
//         case BOX_MOIST:  return BOX_MOIST_BG;
//         case BOX_VOICE:  return BOX_VOICE_BG;
//         default:         return BLACK;
//     }
// }

// // Get box label
// const char* getBoxLabel(int boxIndex) {
//     switch (boxIndex) {
//         case BOX_TEMP:   return "TEMP";
//         case BOX_HUMID:  return "HUMID";
//         case BOX_MOIST:  return "MOIST";
//         case BOX_VOICE:  return "VOICE";
//         default:         return "";
//     }
// }

// // Get box value suffix
// const char* getBoxSuffix(int boxIndex) {
//     switch (boxIndex) {
//         case BOX_TEMP:   return "C";
//         case BOX_HUMID:  return "%";
//         case BOX_MOIST:  return "";
//         case BOX_VOICE:  return "";
//         default:         return "";
//     }
// }

// // Draw a single box with label and value
// void drawBox(int boxIndex, const String& value) {
//     int x, y;
//     getBoxCoords(boxIndex, &x, &y);
//     uint16_t bgColor = getBoxColor(boxIndex);
//     const char* label = getBoxLabel(boxIndex);
//     const char* suffix = getBoxSuffix(boxIndex);

//     // Draw background
//     tft.fillRect(x + 2, y + 2, BOX_W - 4, BOX_H - 4, bgColor);

//     // Draw border
//     tft.drawRect(x, y, BOX_W, BOX_H, WHITE);
//     tft.drawRect(x + 1, y + 1, BOX_W - 2, BOX_H - 2, WHITE);

//     // Draw label at top
//     tft.setTextColor(WHITE);
//     tft.setTextSize(2);
//     int labelX = x + (BOX_W - strlen(label) * 12) / 2;
//     tft.setCursor(labelX, y + 15);
//     tft.print(label);

//     // Draw value in center
//     tft.setTextSize(4);
//     String displayVal = value + suffix;
//     int valX = x + (BOX_W - displayVal.length() * 24) / 2;
//     int valY = y + BOX_H / 2 - 16;
//     tft.setCursor(valX, valY);
//     tft.print(displayVal);
// }

// // Draw all boxes
// void drawAllBoxes() {
//     drawBox(BOX_TEMP, currentTemp);
//     drawBox(BOX_HUMID, currentHumid);
//     drawBox(BOX_MOIST, currentMoist);
//     drawBox(BOX_VOICE, currentVoice);
// }

// // Parse and handle serial command
// void handleCommand(const String& line) {
//     if (line.length() < 2) return;

//     // Temperature: "S T 23.7"
//     if (line.startsWith("S T ")) {
//         currentTemp = line.substring(4);
//         currentTemp.trim();
//         drawBox(BOX_TEMP, currentTemp);
//         Serial.println("OK TEMP");
//     }
//     // Humidity: "S H 41"
//     else if (line.startsWith("S H ")) {
//         currentHumid = line.substring(4);
//         currentHumid.trim();
//         drawBox(BOX_HUMID, currentHumid);
//         Serial.println("OK HUMID");
//     }
//     // Moisture: "S M 78"
//     else if (line.startsWith("S M ")) {
//         currentMoist = line.substring(4);
//         currentMoist.trim();
//         drawBox(BOX_MOIST, currentMoist);
//         Serial.println("OK MOIST");
//     }
//     // Voice: "V LIGHTS ON"
//     else if (line.startsWith("V ")) {
//         currentVoice = line.substring(2);
//         currentVoice.trim();
//         if (currentVoice.length() > 10) {
//             // For long text, use smaller font handled in drawBox
//             currentVoice = currentVoice.substring(0, 10);
//         }
//         drawBox(BOX_VOICE, currentVoice);
//         Serial.println("OK VOICE");
//     }
//     else {
//         Serial.print("ERR Unknown: ");
//         Serial.println(line);
//     }
// }

// void setup() {
//     Serial.begin(115200);

//     // Initialize display
//     tft.reset();
//     uint16_t id = tft.readID();
//     if (id == 0x00D3) id = 0x9481;  // Common fix
//     tft.begin(id);
//     tft.setRotation(1);  // Landscape
//     tft.fillScreen(BLACK);

//     // Draw initial UI
//     drawAllBoxes();

//     Serial.println("LCD Ready");
// }

// void loop() {
//     // Read serial data
//     while (Serial.available()) {
//         char c = Serial.read();

//         if (c == '\n') {
//             // Process complete line
//             serialBuffer.trim();
//             if (serialBuffer.length() > 0) {
//                 handleCommand(serialBuffer);
//             }
//             serialBuffer = "";
//         }
//         else if (c != '\r') {
//             serialBuffer += c;

//             // Prevent buffer overflow
//             if (serialBuffer.length() > 64) {
//                 serialBuffer = "";
//             }
//         }
//     }
// }

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
// Pins
#define LCD_RD A0
#define LCD_WR A1
#define LCD_CD A2
#define LCD_CS A3
#define LCD_RESET A4
// Colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
MCUFRIEND_kbv tft;

// Current sensor values
int currentTemp = 0;
int currentHumid = 0;
int currentMoist = 0;

// Serial buffer
String serialBuffer = "";

// Forward declarations
void show(uint16_t bgColor, const char *faces[], const char *messages[], int numItems);
void healthy();
void unhealthy();
void stats();
void handleCommand(const String &line);
void printWrappedText(const char *text, int boxX, int boxY, int boxW, int boxH);
// Mood state based on moisture
bool moistureIsBad = false;

void setup()
{
  Serial.begin(115200); // Must match Python SerialBridge baudrate
  tft.reset();          // resets the hardware
  tft.begin(0x9481);    // starts up the screen
  tft.setRotation(1);   // sets the rotation of the screen

  tft.fillScreen(WHITE);

  // Show initial display
  healthy();
  stats();

  Serial.println("LCD Ready");
}

void loop()
{
  // Read serial data
  while (Serial.available())
  {
    char c = Serial.read();

    if (c == '\n')
    {
      // Process complete line
      serialBuffer.trim();
      if (serialBuffer.length() > 0)
      {
        handleCommand(serialBuffer);
      }
      serialBuffer = "";
    }
    else if (c != '\r')
    {
      serialBuffer += c;

      // Prevent buffer overflow
      if (serialBuffer.length() > 64)
      {
        serialBuffer = "";
      }
    }
  }
}

// Parse and handle serial command
void handleCommand(const String &line)
{
  if (line.length() < 2)
    return;

  // Temperature: "S T 23"
  if (line.startsWith("S T "))
  {
    currentTemp = line.substring(4).toInt();
    stats();
    Serial.println("OK TEMP");
  }
  // Humidity: "S H 65"
  else if (line.startsWith("S H "))
  {
    currentHumid = line.substring(4).toInt();
    stats();
    Serial.println("OK HUMID");
  }
  else if (line.startsWith("S M "))
  {
    currentMoist = line.substring(4).toInt();

    // Determine mood from moisture category
    bool newBad = (currentMoist < 1000); // BAD if under 1000, else happy for average+good

    // Only change the big face/message when category changes
    if (newBad != moistureIsBad)
    {
      moistureIsBad = newBad;

      if (moistureIsBad)
      {
        unhealthy(); // sad faces + sad messages
      }
      else
      {
        healthy(); // happy faces + happy messages
      }
    }

    // Always update the stats boxes
    stats();
    Serial.println("OK MOIST");
  }

  // Healthy state: "H"
  else if (line.equals("H"))
  {
    healthy();
    stats();
    Serial.println("OK HEALTHY");
  }
  // Unhealthy state: "U"
  else if (line.equals("U"))
  {
    unhealthy();
    stats();
    Serial.println("OK UNHEALTHY");
  }
  else
  {
    Serial.print("ERR Unknown: ");
    Serial.println(line);
  }
}
//--------------------------------------------------------------------------------------------------------------------
void show(uint16_t bgColor, const char *faces[], const char *messages[], int numItems)
{

  tft.fillScreen(bgColor);

  int randomIndex = random(numItems);

  const char *face = faces[randomIndex];
  const char *message = messages[randomIndex];

  // -------- FACE BOX --------
  int faceBoxWidth = 100;
  int faceBoxHeight = 100;
  int faceBoxX = 30;
  int faceBoxY = 120;

  tft.fillRect(faceBoxX, faceBoxY, faceBoxWidth, faceBoxHeight, BLACK);

  tft.setTextSize(4);
  tft.setTextColor(WHITE);

  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(face, 0, 0, &x1, &y1, &w, &h);

  int textX = faceBoxX + (faceBoxWidth - w) / 2;
  int textY = faceBoxY + (faceBoxHeight - h) / 2;

  tft.setCursor(textX, textY);
  tft.print(face);

  // -------- MESSAGE BOX --------
  int msgBoxWidth = 300;
  int msgBoxHeight = 150;
  int msgBoxX = 160;
  int msgBoxY = 120;

  tft.fillRect(msgBoxX, msgBoxY, msgBoxWidth, msgBoxHeight, WHITE);
  tft.drawRect(msgBoxX, msgBoxY, msgBoxWidth, msgBoxHeight, BLACK);

  printWrappedText(message, msgBoxX, msgBoxY, msgBoxWidth, msgBoxHeight);
}

void healthy()
{

  const char *faces[] = {
      "n_n",
      "^_^",
      "o_o",
      ">_<"};

  const char *messages[] = {
      "Hydrated and glowing, just like you!",
      "Sending you both lots of love <3",
      "Small steps still count!",
      "Thinking of you..."};

  show(GREEN, faces, messages, 4);
}

//--------------------------------------------------------------------------------------------------------------------
void unhealthy()
{

  const char *faces[] = {
      "T_T",
      ";_;",
      "x_x",
      ">_<"};

  const char *messages[] = {
      "Feeling a little dry... still love you though.",
      "A bit thirsty, but I know you care.",
      "Low energy today... send water please.",
      "Missing some sunshine and love."};

  show(RED, faces, messages, 4);
}

const char *moistureLabel(int value)
{
  if (value > 2000)
  {
    return "GOOD";
  }
  else if (value >= 1000)
  {
    return "AVERAGE";
  }
  else
  {
    return "BAD";
  }
}

//--------------------------------------------------------------------------------------------------------------------
void stats()
{

  // 3 equal boxes at top: 480px width / 3 = 160px each, with margins
  int boxWidth = 140;
  int boxHeight = 70;
  int boxY = 20;
  int spacing = 20;
  int startX = 20;

  int16_t x1, y1;
  uint16_t w, h;

  // ---------- TEMPERATURE BOX ----------
  int tempBoxX = startX;
  tft.fillRect(tempBoxX, boxY, boxWidth, boxHeight, WHITE);
  tft.drawRect(tempBoxX, boxY, boxWidth, boxHeight, BLACK);

  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(tempBoxX + 35, boxY + 8);
  tft.print("TEMP");

  char tempBuffer[20];
  sprintf(tempBuffer, "%d%cC", currentTemp, 247);
  tft.setTextSize(3);
  tft.getTextBounds(tempBuffer, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(tempBoxX + (boxWidth - w) / 2, boxY + 35);
  tft.print(tempBuffer);

  // ---------- HUMIDITY BOX ----------
  int humidBoxX = startX + boxWidth + spacing;
  tft.fillRect(humidBoxX, boxY, boxWidth, boxHeight, WHITE);
  tft.drawRect(humidBoxX, boxY, boxWidth, boxHeight, BLACK);

  tft.setTextSize(2);
  tft.setCursor(humidBoxX + 25, boxY + 8);
  tft.print("HUMID");

  char humidBuffer[20];
  sprintf(humidBuffer, "%d%%", currentHumid);
  tft.setTextSize(3);
  tft.getTextBounds(humidBuffer, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(humidBoxX + (boxWidth - w) / 2, boxY + 35);
  tft.print(humidBuffer);

  // ---------- MOISTURE BOX ----------
  int moistBoxX = startX + 2 * (boxWidth + spacing);
  tft.fillRect(moistBoxX, boxY, boxWidth, boxHeight, WHITE);
  tft.drawRect(moistBoxX, boxY, boxWidth, boxHeight, BLACK);

  tft.setTextSize(2);
  tft.setCursor(moistBoxX + 25, boxY + 8);
  tft.print("MOIST");

  const char *label = moistureLabel(currentMoist);

  tft.setTextSize(3);
  tft.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(moistBoxX + (boxWidth - w) / 2, boxY + 35);
  if (currentMoist > 2000)
  {
    tft.setTextColor(GREEN);
  }
  else if (currentMoist >= 1000)
  {
    tft.setTextColor(YELLOW);
  }
  else
  {
    tft.setTextColor(RED);
  }

  tft.print(label);
  tft.setTextColor(BLACK);
}

//--------------------------------------------------------------------------------------------------------------------
void printWrappedText(const char *text, int boxX, int boxY, int boxW, int boxH)
{

  tft.setTextSize(3.5);
  tft.setTextColor(BLACK);

  int margin = 12;
  int cursorX = boxX + margin;
  int cursorY = boxY + margin;

  int maxX = boxX + boxW - margin;
  int maxY = boxY + boxH - margin;

  char word[40]; // holds one word at a time
  int wordIndex = 0;

  for (int i = 0;; i++)
  {

    char c = text[i];

    if (c == ' ' || c == '\0')
    {

      word[wordIndex] = '\0'; // end word

      int16_t x1, y1;
      uint16_t w, h;

      tft.getTextBounds(word, 0, 0, &x1, &y1, &w, &h);

      // Wrap if needed
      if (cursorX + w > maxX)
      {
        cursorX = boxX + margin;
        cursorY += h + 6;
      }

      // Stop if exceeding box height
      if (cursorY + h > maxY)
      {
        return;
      }

      tft.setCursor(cursorX, cursorY);
      tft.print(word);
      tft.print(" ");

      cursorX += w + 6;
      wordIndex = 0;

      if (c == '\0')
        break;
    }
    else
    {
      if (wordIndex < 39)
      { // prevent overflow
        word[wordIndex++] = c;
      }
    }
  }
}