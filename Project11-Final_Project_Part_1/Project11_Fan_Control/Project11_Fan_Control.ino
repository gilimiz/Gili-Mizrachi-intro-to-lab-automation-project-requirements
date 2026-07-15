/*
  Project 11 - Requirement 1: Fan control with servo
  This sketch moves the mini fan mounted on the servo horn to the "on" position
  and reports the current angle over Serial.
*/

#include <Servo.h>

const int SERVO_PIN = 7;          // Servo signal pin connected to D7
const int FAN_ON_ANGLE = 0;       // Fixed on position for the fan
const int FAN_OFF_ANGLE = 180;    // Optional off position if needed later
const unsigned long STATUS_INTERVAL_MS = 2000;

Servo fanServo;
unsigned long lastStatusTime = 0;
int currentAngle = FAN_ON_ANGLE;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for Serial port to open
  }

  Serial.println("Project 11 - Fan control started");
  fanServo.attach(SERVO_PIN);

  setFanAngle(FAN_ON_ANGLE);
  Serial.println("Fan moved to ON position");
}

void loop() {
  unsigned long now = millis();
  if (now - lastStatusTime >= STATUS_INTERVAL_MS) {
    lastStatusTime = now;
    reportStatus();
  }
}

void setFanAngle(int angle) {
  currentAngle = constrain(angle, 0, 180);
  fanServo.write(currentAngle);
}

void reportStatus() {
  Serial.print("Fan servo angle: ");
  Serial.print(currentAngle);
  Serial.println(" degrees");
}
