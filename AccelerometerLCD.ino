//Included Libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"

//Pin definitions
const int x_out = A2;
const int y_out = A1;
const int z_out = A0;
#define TFT_DC 7
#define TFT_CS 10
//TFT_SCL 13
//TFT_SDA 11
//TFT_RES 8
//TFT_BLK 9 or just 3.3V


//Creation of LCD object to control
Adafruit_GC9A01A tft(TFT_CS, TFT_DC);


void setup() {
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Accelerometer Test");

  tft.begin();

  Serial.println(testText());
}

void loop() {
    for(uint8_t rotation=0; rotation<4; rotation++) {
    tft.setRotation(rotation);
    testText();
    delay(1000);
    Serial.print("x value: ");
    Serial.println(analogRead(x_out));
    Serial.print("Y value: ");
    Serial.println(analogRead(y_out));
    Serial.print("Z value: ");
    Serial.println(analogRead(z_out));
    Serial.println("");
  }
}

unsigned long testText() {
  tft.fillScreen(GC9A01A_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(GC9A01A_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(GC9A01A_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(GC9A01A_GREEN);
  tft.setTextSize(5);
  tft.println("BASIC");
  tft.setTextSize(2);
  tft.println("have a read of this");
  tft.setTextSize(1);
  tft.println("isn't actually doing much.");
  tft.println("will update for displaying Accelerometer values later");
  return micros() - start;
}
