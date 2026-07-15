/*
  Project 11 - Requirement 2: Fan control based on accelerometer
  - Fan power enable output: D7
  - Servo signal: D3
  - Accelerometer: I2C

  The servo angle follows the accelerometer X-axis continuously,
  and the fan power remains enabled while running.
*/

#include "Arduino_SensorKit.h"
#include <Servo.h>

const int FAN_POWER_PIN = 7;
const int SERVO_PIN = 3;
const int STATUS_INTERVAL_MS = 500;

const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 170;
const int FAN_ACTIVE_MAX_ANGLE = 45;

Servo fanServo;
int currentServoAngle = 90;
unsigned long lastStatusTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println("Project 11 - Acceleration fan control started");

  pinMode(FAN_POWER_PIN, OUTPUT);

  fanServo.attach(SERVO_PIN);
  setFanPower(true);
  setServoAngle(90);

  Accelerometer.begin();
}

void loop() {
  updateServoFromAccelerometer();

  unsigned long now = millis();
  if (now - lastStatusTime >= STATUS_INTERVAL_MS) {
    lastStatusTime = now;
    reportStatus();
  }
}

void setFanPower(bool enabled) {
  digitalWrite(FAN_POWER_PIN, enabled ? HIGH : LOW);
}

void setServoAngle(int angle) {
  currentServoAngle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  fanServo.write(currentServoAngle);
  setFanPower(currentServoAngle <= FAN_ACTIVE_MAX_ANGLE);
}

void reportStatus() {
  Serial.print("Fan: ON | Servo angle: ");
  Serial.print(currentServoAngle);
  Serial.println(" degrees");
}

void updateServoFromAccelerometer() {
  float xValue = Accelerometer.readX();
  float ratio = constrain(xValue, -1.0f, 1.0f);
  int mappedAngle = (int)((ratio + 1.0f) * 85.0f);
  setServoAngle(mappedAngle);

  Serial.print("Accelerometer X=");
  Serial.print(xValue);
  Serial.print(" | Ratio=");
  Serial.print(ratio);
  Serial.print(" | Servo angle=");
  Serial.println(currentServoAngle);
}
