#include <Servo.h>

const int rotaryPin = A0;
const int servoPin = 7;

Servo myServo;

void setup()
{
  Serial.begin(9600);
  myServo.attach(servoPin);
}

void loop()
{
  static int lastAngle = -1;

  int rotaryValue = analogRead(rotaryPin);
  int angle = map(rotaryValue, 0, 1023, 0, 180);

  if (angle != lastAngle)
  {
    myServo.write(angle);

    Serial.print("Rotary: ");
    Serial.print(rotaryValue);
    Serial.print(" -> Angle: ");
    Serial.println(angle);

    lastAngle = angle;
  }

  delay(50);
}

