# Project 4: measuring overhead of digitalWrite() with 2 pins

1. Comparing different types of time overhead

## write a program that does the following:
- copy your code from project 3 to project 4V
- Blink an LED on pin 13 with delay 1 msV
- Blink an LED on pin 12 with delay 1 ms, in this case there isn't actually a LED connected to this pin, but we can still use it to measure the overhead of the digitalWrite() function, using the logic analyzer.V
- both leds should be HIGH, then delay, then both leds LOW, then delayV
- connect the pins to two inputs in the logic analyzer and don't forget to add the ground connection from the Arduino to the logic analyzer.V

## Exercise 1
- measure the delay between the two digitalWrite() functions using the logic analyzer.
Paste screenshots below:
![delay between](<4 - difference between-1.png>)
enter the delay in usec here:  _3.593_________

## write a 2nd program that does the following:
- based on the first program, add any calculation (adding one to an additional variable for example) and store the result in a variable between the two digitalWrite() functions.

## Exercise 2
- measure the delay the originated from the calculation between the two digitalWrite() functions using the logic analyzer.
Paste screenshots below:
![delay](<4 - difference between after add-1.png>)
enter the delay in usec here:  __3.615___

## Exercise 3
- Use chatGPT or similar to find how to write simultaneously to both pins. Measure the delay between the pins now. 
- Paste a screenshot below.
![alt text](<4 - no delay-1.png>)
- Comparison of AI changes if any:
void setBothPins(bool state) {
  if (state) {
    PORTB |= _BV(PB4) | _BV(PB5);  // pin 12 and pin 13 on Arduino Uno/Nano
  } else {
    PORTB &= ~(_BV(PB4) | _BV(PB5));
  }
}
the AI code makes both pins go up or down at the same time

## Git
 - Comparison of AI changes if any:
 - Commit and push the two programs and the README into the repository

