const int ledPin = 4;
const int buttonPin = 2;

volatile bool ledOn = false;
volatile unsigned long ledOnTime = 0;

void handleButtonPress()
{
  ledOn = true;
  ledOnTime = millis();
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);
}

void loop()
{
  if (ledOn && (millis() - ledOnTime >= 5000UL))
  {
    ledOn = false;
    digitalWrite(ledPin, LOW);
  }

  if (ledOn)
  {
    digitalWrite(ledPin, HIGH);
  }
}
