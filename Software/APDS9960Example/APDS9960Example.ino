/*
  APDS9960 - All sensor data from APDS9960

  This example reads all data from the on-board APDS9960 sensor of the
  Nano 33 BLE Sense:
   - color RGB (red, green, blue)
   - proximity
   - gesture
  and prints updates to the Serial Monitor every 100 ms.

  The circuit:
  - Arduino Nano 33 BLE Sense

  This example code is in the public domain.
*/

#include <Arduino_APDS9960.h>

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor to open

  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor.");
    while (true){
      digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  delay(250);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  delay(250);
      // Stop forever
    }
  }

  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  delay(500);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}

int proximity = 0;
int r = 0, g = 0, b = 0;
unsigned long lastUpdate = 0;

void loop() {

  // Check if a proximity reading is available.
  if (APDS.proximityAvailable()) {
    proximity = APDS.readProximity();
  }

  // check if a gesture reading is available
  if (APDS.gestureAvailable()) {
    int gesture = APDS.readGesture();
    switch (gesture) {
      case GESTURE_UP:
        Serial.println("Detected UP gesture");
        digitalWrite(5, HIGH);
        digitalWrite(6, LOW);
        break;

      case GESTURE_DOWN:
        Serial.println("Detected DOWN gesture");
        digitalWrite(6, HIGH);
        digitalWrite(5, LOW);
        break;

      case GESTURE_LEFT:
        Serial.println("Detected LEFT gesture");
        break;

      case GESTURE_RIGHT:
        Serial.println("Detected RIGHT gesture");
        break;

      default:
        // ignore
        break;
    }
  }

  // check if a color reading is available
  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b);
  }

  // Print updates every 100ms
  if (millis() - lastUpdate > 100) {
    lastUpdate = millis();
    Serial.print("PR=");
    Serial.print(proximity);
    Serial.print(" rgb=");
    Serial.print(r);
    Serial.print(",");
    Serial.print(g);
    Serial.print(",");
    Serial.println(b);
  }
}
