/*
  Project 11 - Requirement 1: Fan control with servo
  This sketch moves the mini fan mounted on the servo horn to the "on" position
  and reports the current angle over Serial.
*/

#include <Wire.h>
#include <Servo.h>

const int SERVO_PIN = 3;          // Servo signal pin connected to D3
const int FAN_POWER_PIN = 7;      // Fan enable pin connected to D7
const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 170;
const int FAN_ON_ANGLE = 90;
const unsigned long ACCEL_UPDATE_INTERVAL_MS = 100;

const uint8_t ACCEL_ADDR_1 = 0x1D;
const uint8_t ACCEL_ADDR_2 = 0x53;
const uint8_t ACCEL_DEVID_REG = 0x00;
const uint8_t ADXL345_DEVID = 0xE5;
const uint8_t ACCEL_POWER_CTL = 0x2D;
const uint8_t ACCEL_DATA_FORMAT = 0x31;
const uint8_t ACCEL_BW_RATE = 0x2C;
const uint8_t ACCEL_DATAX0 = 0x32;

Servo fanServo;
unsigned long lastAccelUpdateTime = 0;
uint8_t accelAddress = 0;
int currentAngle = SERVO_MIN_ANGLE;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for Serial port to open
  }

  Serial.println("Project 11 - Fan control started");
  pinMode(FAN_POWER_PIN, OUTPUT);
  digitalWrite(FAN_POWER_PIN, LOW);
  fanServo.attach(SERVO_PIN);

  setFanAngle(currentAngle);
  Serial.println("Fan movement sequence started");

  Wire.begin();
  accelAddress = detectAccelerometer();
  if (accelAddress == 0) {
    Serial.println("Accelerometer not found on I2C bus.");
    Serial.println("Please check wiring and power.");
  } else {
    Serial.print("Accelerometer found at 0x");
    Serial.println(accelAddress, HEX);
    initializeAccelerometer(accelAddress);
  }
}

void loop() {
  unsigned long now = millis();
  if (accelAddress != 0 && now - lastAccelUpdateTime >= ACCEL_UPDATE_INTERVAL_MS) {
    lastAccelUpdateTime = now;
    updateServoFromAccelerometer();
    updateFanPower();
    reportStatus();
  }
}

void setFanAngle(int angle) {
  currentAngle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  fanServo.write(currentAngle);
}

void updateFanPower() {
  if (currentAngle == FAN_ON_ANGLE) {
    digitalWrite(FAN_POWER_PIN, HIGH);
  } else {
    digitalWrite(FAN_POWER_PIN, LOW);
  }
}

void reportStatus() {
  Serial.print("Fan servo angle: ");
  Serial.print(currentAngle);
  Serial.println(" degrees");
}

uint8_t detectAccelerometer() {
  uint8_t candidates[] = {ACCEL_ADDR_1, ACCEL_ADDR_2};
  for (uint8_t i = 0; i < sizeof(candidates); i++) {
    uint8_t address = candidates[i];
    Wire.beginTransmission(address);
    Wire.write(ACCEL_DEVID_REG);
    if (Wire.endTransmission(false) != 0) {
      continue;
    }
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() < 1) {
      continue;
    }
    uint8_t devid = Wire.read();
    if (devid == ADXL345_DEVID) {
      return address;
    }
  }
  return 0;
}

void initializeAccelerometer(uint8_t address) {
  writeRegister(address, ACCEL_POWER_CTL, 0x08);
  writeRegister(address, ACCEL_DATA_FORMAT, 0x08);
  writeRegister(address, ACCEL_BW_RATE, 0x0A);
}

void updateServoFromAccelerometer() {
  int16_t rawX, rawY, rawZ;
  if (!readAccelData(rawX, rawY, rawZ)) {
    return;
  }
  float xG = rawX * 0.004;
  float normalized = constrain(xG, -1.0, 1.0);
  int mappedAngle = mapFloatToAngle(normalized);
  setFanAngle(mappedAngle);
}

bool readAccelData(int16_t &x, int16_t &y, int16_t &z) {
  Wire.beginTransmission(accelAddress);
  Wire.write(ACCEL_DATAX0);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  Wire.requestFrom(accelAddress, (uint8_t)6);
  if (Wire.available() < 6) {
    return false;
  }
  x = Wire.read() | (Wire.read() << 8);
  y = Wire.read() | (Wire.read() << 8);
  z = Wire.read() | (Wire.read() << 8);
  return true;
}

int mapFloatToAngle(float normalized) {
  float angle = (normalized + 1.0) * (SERVO_MAX_ANGLE / 2.0);
  return constrain((int)angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
