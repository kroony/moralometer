#include <Stepper.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        5 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 42 //
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// How many NeoPixels are attached to the Arduino?


const int stepperMaxPosition = 700;  // change this to fit the number of steps per half-revolution
int stepperCurrentPosition = 0;

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepperMaxPosition, 8, 10, 9, 11);

// Add a global variable for the analog pin and reading
const byte analogPin = A0;
const int analogMaxValue = 850;
int analogInput = 0;
const byte buttonPin = 6;
const byte stepperZeroPin = 4;
const byte moralityInverterSwitchPin = 3;

byte idleFrameCount = 0;

void setup() {
  // set the speed at 20 rpm:
  myStepper.setSpeed(38);
  // initialize the serial port:
  Serial.begin(9600);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(25); //0 - 255

  // Enable internal pullup resistors for the buttons
  pinMode(stepperZeroPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);

  // Enable toggle switches for morality inversion
  pinMode(moralityInverterSwitchPin, INPUT_PULLUP);

  sendStepperToZero();
}

void loop() {
  //set lights to something festive
  for(int i = 0; i< NUMPIXELS; i++)
  {
    if (i % 2 == 0) { pixels.setPixelColor(i, pixels.Color(100, 0, 0));} //evens red
    else { pixels.setPixelColor(i, pixels.Color(0, 100, 0)); } // odds green
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  sendStepperToCenter();
  
  // Get sensor reading
  analogInput = takeReading();

  // Begin "calculating" mode
  calculate();

  // Enter "results" mode
  displayResults();
}

void sendStepperToZero() {
  
  // Keep stepping backwards until we hit the zero switch
  while(digitalRead(stepperZeroPin) == HIGH) {
    myStepper.step(-1);
  }
  
  stepperCurrentPosition = 0;  // Reset position counter once we're at zero
}

void stepStepperSafely(int steps) {
  if (steps < 0) {
    myStepper.step(stepperCurrentPosition + steps < 0 ? -stepperCurrentPosition : steps);
    stepperCurrentPosition += stepperCurrentPosition + steps < 0 ? -stepperCurrentPosition : steps;
  }
  else if (steps > 0) {
    myStepper.step(stepperCurrentPosition + steps > stepperMaxPosition ? stepperMaxPosition - stepperCurrentPosition : steps);
    stepperCurrentPosition += stepperCurrentPosition + steps > stepperMaxPosition ? stepperMaxPosition - stepperCurrentPosition : steps;
  }
}

void sendStepperToCenter() {
  stepStepperSafely((stepperMaxPosition / 2) - stepperCurrentPosition);
}

int takeReading() {
  // Continue taking readings every 100ms until button press
  while(digitalRead(buttonPin) == HIGH) {  // Read until button is pressed (LOW)
    delay(100);  // Wait 100ms between readings
    idleFrameCount++;
    if(idleFrameCount == 1)
    {
      //red green pattern
      for(int i = 0; i< NUMPIXELS; i++)
      {
        if (i % 2 == 0) { pixels.setPixelColor(i, pixels.Color(100, 0, 0));} //evens red
        else { pixels.setPixelColor(i, pixels.Color(0, 100, 0)); } // odds green
      }
      pixels.show();
    }
    else if(idleFrameCount == 3)
    {
      for(int i = 0; i< NUMPIXELS; i++)
      {
        if (i % 2 == 0) { pixels.setPixelColor(i, pixels.Color(0, 100, 0));} //evens red
        else { pixels.setPixelColor(i, pixels.Color(100, 0, 0)); } // odds green
      }
      pixels.show();
    }
    else if(idleFrameCount >= 10) { idleFrameCount = 0; }
  }
  int reading = analogRead(analogPin);
  Serial.print("Button press detected. Sensor value: ");
  Serial.println(reading);
  return reading;
}

void calculate() {
  // For five seconds we tell the stepper motor to wiggle back and forth, lights to flash, and buzzer to beep
  for (byte i = 0; i < 3; i++) {
    sweepLEDsOverTime(500, true, 255, 0, 0);
    sweepLEDsOverTime(500, false, 255, 0, 0);
  }
}

void sweepLEDsOverTime(int totalTime, bool direction, byte r, byte g, byte b) {
  // We will want to update each LED one at a time, and tell the stepper motor to move enough steps to move to the next LED
  int stepsPerLED = stepperMaxPosition / NUMPIXELS;
  for(int i = 0; i < NUMPIXELS; i++) {
    setLEDsSolidColour(0, 0, 0);
    if (direction) {
      pixels.setPixelColor(NUMPIXELS - i, pixels.Color(map(i,0,NUMPIXELS,0,255), map(NUMPIXELS-i,0,NUMPIXELS,0,255), 0));
    }
    else {
      pixels.setPixelColor(i, pixels.Color(map(NUMPIXELS-i,0,NUMPIXELS,0,255), map(i,0,NUMPIXELS,0,255), b));
    }
    pixels.show();
    stepStepperSafely(direction == true ? stepsPerLED : -stepsPerLED);
    delay(totalTime / NUMPIXELS);
  }
}

void displayResults() {
  // Map sensor value (0-analogMaxValue) to naughty/middle/nice (0 to 2)
  byte moralityResult = map(analogInput, 0, analogMaxValue, 0, 2);
  // Invert morality if the switch is toggled
  if(digitalRead(moralityInverterSwitchPin) == LOW) {
    moralityResult = 2 - moralityResult;
  }
  
  // Nice
  if(moralityResult == 0) {
    setLEDsSolidColour(0, 255, 0);
    stepStepperSafely(-stepperMaxPosition);
  }
  // Middle
  else if (moralityResult == 1) {
    setLEDsSolidColour(255, 255, 0);
    sendStepperToCenter();
  }
  // Naughty
  else {
    setLEDsSolidColour(255, 0, 0);
    stepStepperSafely(stepperMaxPosition);
  }
  
  delay(3000);  // Show result for 3 seconds
}

void setLEDsSolidColour(byte r, byte g, byte b)
{
  for(int i = 0; i< NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
