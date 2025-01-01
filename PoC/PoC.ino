
// PoC.ino
// Copyright © 2018-2025 Erik Baauw. All rights reserved.
//
// First proof of concept for ceiling fan controller.

// Arduino Relay Shield v3.0 PIN assignments.
#define RELAY1       7
#define RELAY2       6
#define RELAY3       5
#define RELAY4       4
#define ONBOARD_LED 13

// ***** Wiring ****************************************************************

#define RELAY_OFF_LOW     RELAY4    // NC: off            NO: low speed
#define RELAY_MEDIUM_HIGH RELAY3    // NC: medium speed   NO: high speed
#define RELAY_POWER       RELAY2    // NC: RELAY_OFF_LOW  NO: RELAY_MEDIUM_HIGH
#define RELAY_DIRECTION   RELAY1    // NC: down           NO: up
#define BUTTON_SPEED      2
#define BUTTON_DIRECTION  3

// ***** Fan speed control *****************************************************

#define OFF 0
#define LOW_SPEED 1
#define MEDIUM_SPEED 2
#define HIGH_SPEED 3

void setSpeed(int speed) {
  switch (speed) {
    case OFF:
      Serial.print("off\n");
      digitalWrite(RELAY_OFF_LOW, LOW);       // off
      digitalWrite(RELAY_POWER, LOW);         // power RELAY_OFF_LOW
      digitalWrite(RELAY_MEDIUM_HIGH, LOW);   // not powered
      break;
    case LOW_SPEED:
      Serial.print("low speed\n");
      digitalWrite(RELAY_OFF_LOW, HIGH);      // low speed
      digitalWrite(RELAY_POWER, LOW);         // power RELAY_OFF_LOW
      digitalWrite(RELAY_MEDIUM_HIGH, LOW);   // not powered
      break;
    case MEDIUM_SPEED:
      Serial.print("medium speed\n");
      digitalWrite(RELAY_MEDIUM_HIGH, LOW);   // medium speed
      digitalWrite(RELAY_POWER, HIGH);        // power RELAY_MEDIUM_HIGH
      digitalWrite(RELAY_OFF_LOW, LOW);       // not powered
      break;
    case HIGH_SPEED:
      Serial.print("high speed\n");
      digitalWrite(RELAY_MEDIUM_HIGH, HIGH);  // high speed
      digitalWrite(RELAY_POWER, HIGH);        // power RELAY_MEDIUM_HIGH
      digitalWrite(RELAY_OFF_LOW, LOW);       // not powered
      break;
  }
}

void setupSpeed() {
  pinMode(RELAY_OFF_LOW, OUTPUT);
  pinMode(RELAY_MEDIUM_HIGH, OUTPUT);
  pinMode(RELAY_POWER, OUTPUT);
}

// ***** Fan direction control *************************************************

#define DOWN 0
#define UP 1

void setDirection(int direction) {
  switch (direction) {
    case DOWN:
      Serial.print("blowing down\n");
      digitalWrite(RELAY_DIRECTION, LOW);     // down
      break;
    case UP:
      Serial.print("blowing up\n");
      digitalWrite(RELAY_DIRECTION, HIGH);    // up
      break;
  }
}

void setupDirection() {
  pinMode(RELAY_DIRECTION, OUTPUT);
}

// ***** Button handler ******************************************************

#define NONE    0
#define PRESS   1
#define RELEASE 2

bool buttonReleased[32];

void setupButton(int button) {
  pinMode(button, INPUT);
  buttonReleased[button] = true;
}

int buttonEvent(int button) {
  if (digitalRead(button) == HIGH) {
    if (buttonReleased[button]) {
      buttonReleased[button] = false;
      Serial.print("button pressed\n");
      return PRESS;
    }
  } else if (!buttonReleased[button]) {
    buttonReleased[button] = true;
    Serial.print("button released\n");
    return RELEASE;
  }
  return NONE;
}

// ***** Main Program ********************************************************

int speed = OFF;
int direction = DOWN;
int buttonSpeedReleased = true;
int buttonDirectionReleased = true;

void setup() {
  Serial.begin(9600);
  Serial.print("Ceiling fan PoC\n");
  setupSpeed();
  setupDirection();
  setupButton(BUTTON_SPEED);
  setupButton(BUTTON_DIRECTION);
  pinMode(ONBOARD_LED, OUTPUT);
}

void loop() {
  if (buttonEvent(BUTTON_SPEED) == PRESS) {
    speed += 3;
    speed %= 4;
    setSpeed(speed);
  }
  if (buttonEvent(BUTTON_DIRECTION) == PRESS) {
    direction = 1 - direction;
    setDirection(direction);
  }
}
