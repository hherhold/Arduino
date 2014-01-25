/*
 * snapOnePicPerSec
 *
 * http://www.vonroth.com/Arduino/NikonIrControl
 */

#include <nikonIrControl.h>

int CameraIrPin = 13;                // LED connected to digital pin 13

void setup()                    // run once, when the sketch starts
{
  pinMode(CameraIrPin, OUTPUT);      // sets the digital pin as output
}

void loop()                     // run over and over again
{
  cameraSnap(CameraIrPin);
  delay(1000);                  // waits for a second
}
