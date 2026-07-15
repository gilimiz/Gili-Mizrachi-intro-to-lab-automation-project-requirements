/*
  Project 11 - Requirement 2: Fan control based on accelerometer
  - Fan power enable output: D7
  - Servo signal: D3
  - Start/stop button: D2
  - Accelerometer: I2C

  Press the button to toggle the fan on/off. When the fan is enabled,
  the servo angle follows the accelerometer axis and reports status over Serial.
*/

#include <Wire.h>
#include <Servo.h>

const int FAN_POWER_PIN = 7;
const int SERVO_PIN = 3;
const int BUTTON_PIN = 2;
const int STATUS_INTERVAL_MS = 500;
const int BUTTON_DEBOUNCE_MS = 50;

const uint8_t ACCEL_ADDR_1 = 0x1D;
const uint8_t ACCEL_ADDR_2 = 0x53;
const uint8_t ACCEL_DEVID_REG = 0x00;
const uint8_t ADXL345_DEVID = 0xE5;
const uint8_t ACCEL_POWER_CTL = 0x2D;
const uint8_t ACCEL_DATA_FORMAT = 0x31;
const uint8_t ACCEL_BW_RATE = 0x2C;
const uint8_t ACCEL_DATAX0 = 0x32;

const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 180;

Servo fanServo;
uint8_t accelAddress = 0;
int currentServoAngle = 90;
bool fanEnabled = true;
int lastButtonState = LOW;
unsigned long lastButtonDebounceTime = 0;
unsigned long lastStatusTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println("Project 11 - Acceleration fan control started");

  pinMode(FAN_POWER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  fanServo.attach(SERVO_PIN);
  setFanPower(fanEnabled);
  setServoAngle(currentServoAngle);

  Wire.begin();
  accelAddress = detectAccelerometer();
  if (accelAddress == 0) {
    Serial.println("Accelerometer not found on I2C bus.");
    Serial.println("Please check the module wiring and power.");
  } else {
    Serial.print("Accelerometer found at address 0x");
    Serial.println(accelAddress, HEX);
    initializeAccelerometer(accelAddress);
  }
}

void loop() {
  handleButton();

  if (fanEnabled && accelAddress != 0) {
    updateServoFromAccelerometer();
  }

  unsigned long now = millis();
  if (now - lastStatusTime >= STATUS_INTERVAL_MS) {
    lastStatusTime = now;
    reportStatus();
  }
}

void handleButton() {
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState != lastButtonState) {
    lastButtonDebounceTime = millis();
  }

  if ((millis() - lastButtonDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (buttonState == HIGH && lastButtonState == LOW) {
      fanEnabled = !fanEnabled;
      setFanPower(fanEnabled);
      Serial.print("Button pressed, fan ");
      Serial.println(fanEnabled ? "enabled" : "disabled");
    }
  }

  lastButtonState = buttonState;
}

void setFanPower(bool enabled) {
  digitalWrite(FAN_POWER_PIN, enabled ? HIGH : LOW);
}

void setServoAngle(int angle) {
  currentServoAngle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  fanServo.write(currentServoAngle);
}

void reportStatus() {
  Serial.print("Fan: ");
  Serial.print(fanEnabled ? "ON" : "OFF");
  Serial.print(" | Servo angle: ");
  Serial.print(currentServoAngle);
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
  // Set measurement mode
  writeRegister(address, ACCEL_POWER_CTL, 0x08);
  // Use full resolution and +/- 2g range if available
  writeRegister(address, ACCEL_DATA_FORMAT, 0x08);
  // Set output data rate to 100 Hz
  writeRegister(address, ACCEL_BW_RATE, 0x0A);
}

void updateServoFromAccelerometer() {
  int16_t rawX, rawY, rawZ;
  if (!readAccelData(rawX, rawY, rawZ)) {
    return;
  }

  float normalized = normalizeAccelValue(rawX);
  int mappedAngle = mapFloatToAngle(normalized);
  setServoAngle(mappedAngle);
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

float normalizeAccelValue(int16_t raw) {
  float value = raw;
  if (abs(raw) > 1000) {
    value = raw / 8192.0; // ADXL345 16g full-scale estimate
  } else {
    value = raw / 256.0; // ADXL345 2g full-scale estimate
  }
  return constrain(value, -1.0, 1.0);
}

int mapFloatToAngle(float normalized) {
  float angle = (normalized + 1.0) * 90.0;
  return constrain((int)angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
