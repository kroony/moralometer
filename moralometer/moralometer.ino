#include <Stepper.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 21 //
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 21 // Popular NeoPixel ring size

const int stepsPerRevolution = 500;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

// Add a global variable for the analog pin and reading
const int analogPin = A0;
const int analogMaxValue = 850;
int analogInput = 0;
const int buttonPin = 2;

byte idleFrameCount = 0;

void setup() {
  // set the speed at 20 rpm:
  myStepper.setSpeed(50);
  // initialize the serial port:
  Serial.begin(9600);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

void loop() {
  //set lights to something festive
  for(int i = 0; i< NUMPIXELS; i++)
  {
    if (i % 2 == 0) { pixels.setPixelColor(i, pixels.Color(100, 0, 0));} //evens red
    else { pixels.setPixelColor(i, pixels.Color(0, 100, 0)); } // odds green
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  
  // Get sensor reading
  analogInput = takeReading();

  // Begin "calculating" mode
  calculate();

  // Enter "results" mode
  displayResults();
}

int takeReading() {
  // Continue taking readings every 100ms until button press
  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pullup resistor
  
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
    else if(idleFrameCount == 5)
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
  for (int i = 0; i < 5; i++) {
    myStepper.step(stepsPerRevolution);
    trackLightsOverTime(500);
    myStepper.step(-stepsPerRevolution);
    trackLightsOverTime(500);
  }
}

void sweepLEDsOverTime(int totalTime, bool direction, byte r, byte g, byte b) {
  int stepsPerLED = stepsPerRevolution / NUMPIXELS;
  for(int i = 0; i < NUMPIXELS; i++) {
    
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    pixels.show();
    myStepper.step(stepsPerLED * direction);
    delay(totalTime / NUMPIXELS);
  }
}

void displayResults() {
  // Map sensor value (0-analogMaxValue) to steps (-500 to 500)
  int steps = map(analogInput, 0, analogMaxValue, -stepsPerRevolution, stepsPerRevolution);
  
  // Move stepper to position based on sensor reading
  myStepper.step(steps);
  
  // Randomly choose between red and green for the LEDs
  // GOLD PLATING IDEA: play a sound based on the sensor reading
  if(random(2) == 0) {
  } else {
  }
  
  delay(3000);  // Show result for 3 seconds
  
  // Turn off the LEDs
}
