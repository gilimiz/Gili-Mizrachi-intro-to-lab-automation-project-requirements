#include <MD_PWM.h>

const int rotaryPin = A0;
const uint8_t pwmPin = 10;     // PWM-capable pin connected to the LED through your wiring
const uint16_t PWM_FREQ = 50; // Hz

MD_PWM pwm(pwmPin);

void setup()
{
  Serial.begin(9600);

  if (!pwm.begin(PWM_FREQ))
  {
    Serial.println("Failed to initialize PWM");
  }
}

void loop()
{
  static int lastRotaryValue = -1;

  int rotaryValue = analogRead(rotaryPin);

  if (rotaryValue != lastRotaryValue)
  {
    uint8_t duty = map(rotaryValue, 0, 1023, 0, 255);
    pwm.write(duty);

    Serial.print("Rotary: ");
    Serial.print(rotaryValue);
    Serial.print(" -> PWM duty: ");
    Serial.println(duty);

    lastRotaryValue = rotaryValue;
  }

  delay(50);
}