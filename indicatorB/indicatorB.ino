#include <Arduino_APDS9960.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte slaveAddress[6] = "00003";
//TODO sleep mode and IRQ pin interrupts
#define PROX_THRESH 20
#define CLEAR 5
#define BUSY 6

int roomClear = 0;

void setup() {
	Serial.begin(9600);
	while (!Serial); // Wait for serial monitor to open

	boolean error = false;
	int errorDelay = 100;
	if (!APDS.begin()) {
		Serial.println("Error initializing APDS9960 sensor.");
		error = true;
		errorDelay = 50;
	}

	if (!radio.begin()) {
		Serial.println("Error initializing RF24 radio");
		error = true;
		errorDelay = 200;
	} else {
		radio.setDataRate( RF24_250KBPS );
	    radio.openReadingPipe(1, slaveAddress);
	    radio.enableAckPayload();
	    radio.startListening();
	    radio.writeAckPayload(1, &roomClear, sizeof(roomClear)); // pre-load data
	}

	while (error) {
		digitalWrite(CLEAR, HIGH);
		digitalWrite(BUSY, LOW);
		delay(errorDelay);
		digitalWrite(CLEAR, LOW);
		digitalWrite(BUSY, HIGH);
		delay(errorDelay);
	}

	Serial.println("IndicatorB starting");

}

int proximity = 0;
unsigned long lastUpdate = 0;
int currentState = 1;
boolean toggle = false;

void loop() {
	switch (currentState) {
		case 0:
			if (millis() - lastUpdate > 300) {
				if (!toggle) {
					digitalWrite(CLEAR, HIGH);
					digitalWrite(BUSY, HIGH);
					toggle = true;
				} else {
					digitalWrite(CLEAR, LOW);
					digitalWrite(BUSY, LOW);
					toggle = false;
				}
				lastUpdate = millis();
			}

			if (radio.available()) {
				int stat = 0;
				int ret = 200;
		        radio.read( &stat, sizeof(stat) );
		        radio.writeAckPayload(1, &ret, 2);

		        currentState = 1;
		    }
			break;
		case 1:
			if (proximity < PROX_THRESH) {
				if (roomClear) {
					digitalWrite(CLEAR, HIGH);
					digitalWrite(BUSY, LOW);
				} else {
					digitalWrite(CLEAR, LOW);
					digitalWrite(BUSY, HIGH);
				}
			} else { //maybe add a thing where u cant swipe after taking hand away
				digitalWrite(CLEAR, LOW);
				digitalWrite(BUSY, HIGH);
			}

			if (millis() - lastUpdate > 1000) {
				digitalWrite(CLEAR, LOW);
				digitalWrite(BUSY, LOW);
				toggle = false;
			}

			if (radio.available()) {
		        radio.read( &roomClear, sizeof(roomClear) );
		        radio.writeAckPayload(1, &roomClear, sizeof(roomClear));
		    }
			break;
	}

  // Check if a proximity reading is available.
  if (APDS.proximityAvailable()) {
    proximity = APDS.readProximity();
  }

  // check if a gesture reading is available
  if (APDS.gestureAvailable()) {
    int gesture = APDS.readGesture();
    switch (gesture) {
      case GESTURE_UP:
        roomClear = 1;
        radio.writeAckPayload(1, &roomClear, sizeof(roomClear));
        lastUpdate = millis();

        if (currentState > 0) {
	        digitalWrite(BUSY, LOW);
	        digitalWrite(CLEAR, HIGH);
	    }
        break;

      case GESTURE_DOWN:
        roomClear = 0;
        radio.writeAckPayload(1, &roomClear, sizeof(roomClear));
        lastUpdate = millis();

        if (currentState > 0) {
	        digitalWrite(BUSY, HIGH);
	        digitalWrite(CLEAR, LOW);
	    }
        break;
    }
  }
}