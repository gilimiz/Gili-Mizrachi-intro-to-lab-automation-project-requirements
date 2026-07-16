#include <Adafruit_MAX31856.h>

// --- Pin Definitions ---
const int DRDY_PIN = 5;        // Data Ready pin for MAX31856
const int CS_PIN = 4;          // Chip Select pin for MAX31856
const uint8_t HEATER_PIN = 10; // PWM output pin controlling the heating element

// --- Object Initialization ---
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(CS_PIN);

// --- P Controller Parameters ---
const float SETPOINT = 31.0;       // Target temperature in degrees C
const float MAX_TEMP_LIMIT = 35.0; // Absolute safety ceiling in degrees C

// Tune this parameter for your specific hardware setup:
const float Kp = 20.0;             // Proportional Gain

// --- Timing Variables (Strict Fixed Sampling Interval) ---
unsigned long lastSampleTime = 0;
const unsigned long SAMPLE_INTERVAL_MS = 3000; // 3-second sample interval

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); 
  
  Serial.println("Initializing Proportional Temperature Controller...");

  pinMode(HEATER_PIN, OUTPUT);
  analogWrite(HEATER_PIN, 0); // Start with heater off

  pinMode(DRDY_PIN, INPUT);
  if (!maxthermo.begin()) {
    Serial.println("Could not initialize thermocouple.");
    while (1) delay(10);
  }

  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
  maxthermo.setConversionMode(MAX31856_CONTINUOUS);

  Serial.println("System Ready! P Control running every 3 seconds...");
}

void loop() {
  unsigned long currentTime = millis();

  // Execute the control loop strictly at the fixed sample interval
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    
    // Ensure a fresh conversion is ready from the MAX31856
    if (digitalRead(DRDY_PIN) == LOW) {
      float currentTemp = maxthermo.readThermocoupleTemperature();
      lastSampleTime = currentTime; // Update the time step

      // 1. Check hard safety limit first
      if (currentTemp >= MAX_TEMP_LIMIT) {
        analogWrite(HEATER_PIN, 0); // Force heater off instantly
        
        Serial.print("!!! EMERGENCY EXCEEDED 35C LIMIT !!! Current: ");
        Serial.print(currentTemp);
        Serial.println(" C | HEATER OFF");
        return; // Skip the rest of the calculation
      }

      // 2. Proportional Control Logic
      float error = SETPOINT - currentTemp;
      float output = Kp * error;

      // 3. Clamp Output between 0 and 100
      if (output > 100.0) {
        output = 100.0;
      } else if (output < 0.0) {
        output = 0.0;
      }

      // 4. Map the 0-100 control range to Arduino PWM (0-255) and drive the heater
      uint8_t pwmValue = map(output, 0, 100, 0, 255);
      analogWrite(HEATER_PIN, pwmValue);

      // 5. Diagnostics Output
      Serial.print("Temp: ");
      Serial.print(currentTemp);
      Serial.print(" C | Error: ");
      Serial.print(error);
      Serial.print(" | Controller Output (0-100): ");
      Serial.print(output);
      Serial.print(" | PWM Value (0-255): ");
      Serial.println(pwmValue);
    }
  }
}