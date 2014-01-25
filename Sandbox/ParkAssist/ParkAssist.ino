// Garage Parking Assistant


// Uses Aiko event library, located at https://github.com/geekscape/Aiko
#include <AikoEvents.h>
using namespace Aiko;

// PING))) Ultrasonic Distance Sensor from Parallax.
// Uses the Ping library by Caleb Zulawski, located at http://arduino.cc/playground/Code/Ping
#include <Ping.h>
const int PingPin = 8;
const double inMod = 0.0;
const double cmMod = 0.0;
Ping ping = Ping(PingPin, inMod, cmMod);

// LED colors are as follows:
// COLOR    LEDs
// GREEN    2,3
// YELLOW   4,5
// RED      6,7
const int GreenLED1Pin = 2;
const int GreenLED2Pin = 3;
const int YellowLED1Pin = 4;
const int YellowLED2Pin = 5;
const int RedLED1Pin = 6;
const int RedLED2Pin = 7;

const int RedStatusDisplay = 0;
const int YellowStatusDisplay = 1;
const int GreenStatusDisplay = 2;

void setup()
{
  // PING pin is setup when Ping global var is instantiated.

  // Status LEDs.
  pinMode(GreenLED1Pin, OUTPUT);
  pinMode(GreenLED2Pin, OUTPUT);
  pinMode(YellowLED1Pin, OUTPUT);
  pinMode(YellowLED2Pin, OUTPUT);
  pinMode(RedLED1Pin, OUTPUT);
  pinMode(RedLED2Pin, OUTPUT);
  
  // Serial pirt for debugging.
  Serial.begin(115200);
  
  // Event handlers
  Events.addHandler(checkDistance, 100);
}

void setStatusDisplay(int statusDisplayColor)
{
  digitalWrite(GreenLED1Pin, LOW);
  digitalWrite(GreenLED2Pin, LOW);
  digitalWrite(YellowLED1Pin, LOW);
  digitalWrite(YellowLED2Pin, LOW);
  digitalWrite(RedLED1Pin, LOW);
  digitalWrite(RedLED2Pin, LOW);
  
  switch(statusDisplayColor) {
    case RedStatusDisplay: {
      digitalWrite(RedLED1Pin, HIGH);
      digitalWrite(RedLED2Pin, HIGH);
      break;
    }
    case YellowStatusDisplay: {
      digitalWrite(YellowLED1Pin, HIGH);
      digitalWrite(YellowLED2Pin, HIGH);
      break;
    }
    case GreenStatusDisplay: {
      digitalWrite(GreenLED1Pin, HIGH);
      digitalWrite(GreenLED2Pin, HIGH);
      break;
    }
  }
}

void checkDistance()
{
  ping.fire();
  double distanceInInches = ping.inches( );
  
  if ( distanceInInches > 12.0 )
  {
    setStatusDisplay(GreenStatusDisplay);
  }
  else if ( ( distanceInInches <= 12.0 ) && ( distanceInInches > 2.0 ) )
  {
    setStatusDisplay(YellowStatusDisplay);
  }
  else if (distanceInInches <= 2.0)
  {
    setStatusDisplay(RedStatusDisplay);
  }
}

void loop()
{
  Events.loop();
}

