#include <MsTimer2.h>

const int ledPin = 4;          // Built-in LED pin on most Arduino boards
const int buttonPin = 3;        // Interrupt-capable digital pin for the button
const unsigned long debounceMs = 50;

volatile bool buttonPressed = false;
volatile bool ledOn = false;
unsigned long lastButtonChangeMillis = 0;
unsigned long ledOnDurationMs = 1000; // Default on-time if no command has been received yet

void turn_off()
{
  digitalWrite(ledPin, LOW);
  ledOn = false;
  MsTimer2::stop();
  Serial.println("LED turned off.");
}

void handleButtonInterrupt()
{
  buttonPressed = true;
}

bool processSerialCommand()
{
  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.length() == 0) {
    return false;
  }

  bool validNumber = true;
  for (unsigned int i = 0; i < line.length(); i++) {
    if (!isDigit(line.charAt(i))) {
      validNumber = false;
      break;
    }
  }

  if (!validNumber) {
    Serial.print("Error: invalid input. Send only digits followed by a newline. Received: ");
    Serial.println(line);
    return false;
  }

  unsigned long value = line.toInt();
  if (value == 0) {
    Serial.println("Error: duration must be greater than 0 milliseconds.");
    return false;
  }

  ledOnDurationMs = value;
  MsTimer2::set(ledOnDurationMs + 1, turn_off);
  Serial.print("I received: ");
  Serial.println(ledOnDurationMs);
  return true;
}

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);
  MsTimer2::set(ledOnDurationMs + 1, turn_off);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonInterrupt, FALLING);

  Serial.println("Ready. Send an on-time in milliseconds followed by Newline.");
  Serial.println("Press the button to light the LED for the configured duration.");
}

void loop()
{
  if (Serial.available() > 0) {
    processSerialCommand();
  }

  if (buttonPressed) {
    buttonPressed = false;
    unsigned long now = millis();
    if (now - lastButtonChangeMillis >= debounceMs) {
      lastButtonChangeMillis = now;
      if (!ledOn) {
        ledOn = true;
        digitalWrite(ledPin, HIGH);
        MsTimer2::start();
        Serial.print("Button pressed, LED on for ");
        Serial.print(ledOnDurationMs);
        Serial.println(" ms.");
      }
    }
  }
}
