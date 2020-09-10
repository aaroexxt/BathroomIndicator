//IndicatorB.ino by Aaron Becker

#include <Arduino_APDS9960.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/power.h>

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00002", "00003"};
//TODO sleep mode and IRQ pin interrupts
#define PROX_THRESH 20
#define CLEAR 5
#define BUSY 6
#define IRQ 2

const boolean debug = false;

boolean radioListening = false;
void setup() {
	Serial.begin(9600);
	while (!Serial); // Wait for serial monitor to open

	boolean error = false;
	int errorDelay = 100;
	int okDelay = 50;
	if (!APDS.begin()) {
		Serial.println("Error initializing APDS9960 sensor.");
		error = true;
		errorDelay = 50;
	}
	APDS.setGestureSensitivity(85);

	//Power saving stuff
	power_adc_disable();
	power_usart0_disable();

	//Radio stuff
  	radio.begin();
	radio.setPALevel(RF24_PA_MAX); //max because we don't want to lose connection
	radio.setRetries(3,3); // delay, count
	radio.openWritingPipe(addresses[1]);
	radio.openReadingPipe(1, addresses[0]); //set address to recieve data
	radio.maskIRQ(1,1,0);
	attachInterrupt(digitalPinToInterrupt(IRQ), recv, FALLING);
	radioTransmitMode();
	radio.stopListening();

	  pinMode(CLEAR, OUTPUT);
	  pinMode(BUSY, OUTPUT);

	while (error) {
		analogWrite(CLEAR, 32);
		digitalWrite(BUSY, LOW);
		delay(errorDelay);
		digitalWrite(CLEAR, LOW);
		analogWrite(BUSY, 32);
		delay(errorDelay);
	}

	Serial.println("IndicatorA starting");

	analogWrite(CLEAR, 32);
	digitalWrite(BUSY, LOW);
	delay(okDelay);
	digitalWrite(CLEAR, LOW);
	analogWrite(BUSY, 32);
	delay(okDelay);
	analogWrite(CLEAR, 32);
	digitalWrite(BUSY, LOW);
	delay(okDelay);
	digitalWrite(CLEAR, LOW);
	analogWrite(BUSY, 32);
	delay(okDelay);
	digitalWrite(CLEAR, LOW);
	digitalWrite(BUSY, LOW);
	delay(500);

}

int proximity = 0;
unsigned long lastUpdate = 0;
int currentState = 0;
int roomClear = 0;
boolean toggle = false;
unsigned long lastSetBusy = 0;

unsigned long lastRecvTime = 0;
unsigned long lastStatusMillis = 0;

void loop() {
	switch (currentState) {
		case 0:
			if (millis() - lastUpdate > 100) {
				if (!toggle) {
					analogWrite(CLEAR, 32);
					analogWrite(BUSY, 32);
					toggle = true;
				} else {
					digitalWrite(CLEAR, LOW);
					digitalWrite(BUSY, LOW);
					toggle = false;
				}
				send(200);
				lastUpdate = millis();
			}
			break;
		case 1:
			if (proximity < PROX_THRESH) {
				if (roomClear) {
					analogWrite(CLEAR, 32);
					digitalWrite(BUSY, LOW);
				} else {
					digitalWrite(CLEAR, LOW);
					analogWrite(BUSY, 32);
				}
				lastUpdate = millis();
			} else { //maybe add a thing where u cant swipe after taking hand away
				digitalWrite(CLEAR, LOW);
				digitalWrite(BUSY, LOW);
				toggle = true;
			}

			if (millis() - lastUpdate > 1000 && toggle) {
				digitalWrite(CLEAR, LOW);
				digitalWrite(BUSY, LOW);
				toggle = false;
			}

			if ((millis() - lastSetBusy > (1000*60*30)) && (lastSetBusy > 0)) { //1.2million msec = 20 minutes, reset room back to clear
				roomClear = 1;
				lastSetBusy = -1;
				send(roomClear);
				//Don't show lights, do this quietly
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
		    send(roomClear);
		    lastSetBusy = -1;
		    lights(true,2);
		    break;

		  case GESTURE_DOWN:
		    roomClear = 0;
		    send(roomClear);
		    lastSetBusy = millis();
		    lights(false,2);
		    break;
		}
	}
	if (millis() - lastStatusMillis > 500) {
		send(200);
		send(roomClear);
		if (millis() - lastRecvTime >= 15000) {
			currentState = 0;
		}
		lastStatusMillis = millis();
	}
}

void send(int dat) {
	radioTransmitMode();
	/*if (dat != 200) {
		Serial.print("S: ");
		Serial.println(dat);
	}*/
    radio.write(&dat, sizeof(dat));
    radioRecieveMode();
}

void recv() {
	radioRecieveMode();
	if (radio.available()) {
		int med = 0;
		radio.read(&med, sizeof(med));

		/*if (med != 200) {
			Serial.print("R: ");
			Serial.println(med);
		}*/
		if (med < 200) {
			roomClear = med;
			if (roomClear == 0) {
				lastSetBusy = millis();
			}
		}
		lastRecvTime = millis();
		currentState = 1;
	}
}

void radioRecieveMode() {
  if (!radioListening) { //if we're not listening
    radio.startListening();
    radioListening = true;
  }
}

void radioTransmitMode() {
  if (radioListening) {
    radio.stopListening();
    radioListening = false;
  }
}

void lights(boolean state, int count) {
	send(roomClear);
	if (state) {
		analogWrite(CLEAR, 32);
		digitalWrite(BUSY, LOW);
	} else {
		digitalWrite(CLEAR, LOW);
		analogWrite(BUSY, 32);
	}
	send(roomClear);

	for (int i = 0; i<count; i++) {
		delay(100);
		send(roomClear);
		digitalWrite(CLEAR, LOW);
		digitalWrite(BUSY, LOW);
		delay(100);
		send(roomClear);
		if (state) {
			analogWrite(CLEAR, 32);
			digitalWrite(BUSY, LOW);
		} else {
			digitalWrite(CLEAR, LOW);
			analogWrite(BUSY, 32);
		}
	}
}
