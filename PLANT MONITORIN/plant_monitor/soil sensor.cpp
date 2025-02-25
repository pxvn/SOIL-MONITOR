#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int AirValue = 2920;   // Replace with Value_1
const int WaterValue = 1100;  // Replace with Value_2
const int SensorPin = 34;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

void setup() {
  Serial.begin(115200); // Open serial port, set the baud rate to 115200 bps
}

void loop() 
{
  soilMoistureValue = analogRead(SensorPin);  // Read sensor value
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  if (soilmoisturepercent > 100) {
    Serial.println("Soil Moisture: 100%");
  } 
  else if (soilmoisturepercent < 0) {
    Serial.println("Soil Moisture: 0%");
  } 
  else {
    Serial.print("Soil Moisture: ");
    Serial.print(soilmoisturepercent);
    Serial.println("%");
  }
  delay(250);
}
