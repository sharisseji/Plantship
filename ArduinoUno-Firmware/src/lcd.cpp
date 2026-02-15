//Libraries
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Arduino.h>
//Pins
#define LCD_RD A0
#define LCD_WR A1
#define LCD_CD A2
#define LCD_CS A3
#define LCD_RESET A4
//Colors
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
MCUFRIEND_kbv tft;

void setup(){
  tft.reset(); //resets the hardware
  tft.begin(0x9481); //starts up the screen
  tft.setRotation(1); //sets the rotation of the screen
}

void loop (){
  tft.fillScreen(GREEN);
  delay(1000); //delay is put here so that we can fully see the colors
  tft.fillScreen(MAGENTA);
  delay(1000);
  tft.fillScreen(BLUE);
  delay(1000);
  tft.fillRect(80, 140, 321, 60, BLACK);
  delay(1000); //by delaying the loop for 1 second
  tft.fillRect(80, 140, 321, 60, WHITE);
  tft.setCursor(160, 155);
  tft.setTextColor(RED);
  tft.setTextSize(4);
  tft.print("i hate you");
  delay(1000); //by delaying the loop for 1 second
}