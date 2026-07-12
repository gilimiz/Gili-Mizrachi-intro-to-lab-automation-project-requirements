/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/
int ledPin13 = 13;
int ledPin12 = 12;
int delayTimeMs = 1;
int counter = 0;

void setBothPins(bool state) {
  if (state) {
    PORTB |= _BV(PB4) | _BV(PB5);  // pin 12 and pin 13 on Arduino Uno/Nano
  } else {
    PORTB &= ~(_BV(PB4) | _BV(PB5));
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(ledPin13, OUTPUT);
  pinMode(ledPin12, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  // Connect pin 13 and pin 12 to the logic analyzer inputs.
  // Also connect Arduino GND to the logic analyzer GND.
  // This updates both pins in one port write so the timing skew is minimal.
  setBothPins(HIGH);

  int result = counter + 1;
  counter = result;

  delay(delayTimeMs);

  setBothPins(LOW);
  delay(delayTimeMs);
}
