// Distance measuring using PING))) and LCD display.

#include <AikoEvents.h>
using namespace Aiko;

// PING))) Ultrasonic Distance Sensor from Parallax.
// Uses the Ping library by Caleb Zulawski, located at http://arduino.cc/playground/Code/Ping
#include <Ping.h>
const int PingPin = 6;
const double inMod = 0.0;
const double cmMod = 0.0;
Ping ping = Ping(PingPin, inMod, cmMod);

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#define RUNNING_AVERAGE_WINDOW 10
double runningAverageArray[RUNNING_AVERAGE_WINDOW];
int samplesUntilPrimed = RUNNING_AVERAGE_WINDOW;

void setup()
{
  lcd.begin(16, 2);
  //         012345678901
  lcd.print("Inches    cm");
  
  Serial.begin(115200);
  
  Events.addHandler(measureRawDistance, 50);
  
  Events.addHandler(displayAveragedDistance, 100);
}

double getAveragedDistance()
{
  if (samplesUntilPrimed != 0) return -1;
  double accumulator = 0.0;
  for (int i = 0; i < RUNNING_AVERAGE_WINDOW; i++)
  {
    accumulator += runningAverageArray[i];
  }
  return accumulator / RUNNING_AVERAGE_WINDOW;
}

void displayAveragedDistance()
{
  lcd.setCursor(0, 1);
  lcd.print(getAveragedDistance());  
}

void measureRawDistance()
{
  ping.fire();
  double newDistanceInInches = ping.inches();
  
  if (samplesUntilPrimed == 0)
  {
    // Move everybody down 1.
    for (int i = RUNNING_AVERAGE_WINDOW - 1; i > 0; i-- )
    {
      runningAverageArray[i] = runningAverageArray[i-1];
    }
    runningAverageArray[0] = newDistanceInInches;
  }
  else
  {
    // Still priming array.
    runningAverageArray[samplesUntilPrimed] = newDistanceInInches;
    samplesUntilPrimed--;
  }
#if 0  
  lcd.setCursor(0, 1);
  lcd.print(ping.inches());
//  lcd.setCursor(0, 10);
//  lcd.print(ping.centimeters());
#endif
}

void loop()
{
  Events.loop();
}

