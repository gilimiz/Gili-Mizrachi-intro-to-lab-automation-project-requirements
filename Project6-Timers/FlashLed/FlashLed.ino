#include <MsTimer2.h>

const int ledPin = 4;
const int buttonPin = 2;

volatile bool ledOn = false;

void turnOffLed()
{
  digitalWrite(ledPin, LOW);
  ledOn = false;
  MsTimer2::stop();
}

void handleButtonPress()
{
  if (!ledOn)
  {
    ledOn = true;
    digitalWrite(ledPin, HIGH);
    MsTimer2::set(30, turnOffLed);
    MsTimer2::start();
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);
}

void loop()
{
  for (int i = 0; i< 10000; i++){
    Serial.println("calculating...");
}
}

