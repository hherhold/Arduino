// Garage Parking Assistant

// Uses Aiko event library, located at https://github.com/geekscape/Aiko
#include <AikoEvents.h>
using namespace Aiko;

// PING))) Ultrasonic Distance Sensor from Parallax.
// Uses the Ping library by Caleb Zulawski, located at http://arduino.cc/playground/Code/Ping
#include <Ping.h>
const int PingPin = 6;
const double inMod = 0.0;
const double cmMod = 0.0;
Ping ping = Ping(PingPin, inMod, cmMod);

const int GreenLED1Pin  = 5;
const int YellowLED1Pin = 4;
const int RedLED1Pin    = 3;

// The button press is handled by an interrupt. INT0 is on D2.
const int ButtonPin = 0;

const int RedStatusDisplay = 0;
const int YellowStatusDisplay = 1;
const int GreenStatusDisplay = 2;

boolean buttonState = LOW;

void setup()
{
  // PING pin is setup when Ping global var is instantiated.

  // Status LEDs.
  pinMode(GreenLED1Pin, OUTPUT);
  pinMode(YellowLED1Pin, OUTPUT);
  pinMode(RedLED1Pin, OUTPUT);
      
  // Event handlers
  Events.addHandler(checkDistance, 100);
  
  attachInterrupt(0, buttonPressedISR, CHANGE);
}

void buttonPressedISR()
{
  buttonState = !buttonState;
}

void setStatusDisplay(int statusDisplayColor)
{
  digitalWrite(GreenLED1Pin, LOW);
  digitalWrite(YellowLED1Pin, LOW);
  digitalWrite(RedLED1Pin, LOW);
  
  switch(statusDisplayColor) {
    case RedStatusDisplay: {
      digitalWrite(RedLED1Pin, HIGH);
      break;
    }
    case YellowStatusDisplay: {
      digitalWrite(YellowLED1Pin, HIGH);
      break;
    }
    case GreenStatusDisplay: {
      digitalWrite(GreenLED1Pin, HIGH);
      break;
    }
  }
}

// Default distance to stop the car. (In inches.)
double RedDistance = 2.0;
double YellowDistance = RedDistance + 12.0;

void checkDistance()
{
  ping.fire();
  double distanceInInches = ping.inches( );

  if (buttonState == true)
  {
    RedDistance = distanceInInches;
    YellowDistance = RedDistance + 12.0;
  }

  if ( distanceInInches > YellowDistance )
  {
    setStatusDisplay(GreenStatusDisplay);
  }
  else if ( ( distanceInInches <= YellowDistance ) && ( distanceInInches > RedDistance ) )
  {
    setStatusDisplay(YellowStatusDisplay);
  }
  else if (distanceInInches <= RedDistance)
  {
    setStatusDisplay(RedStatusDisplay);
  }
}

void loop()
{
  Events.loop();
}

