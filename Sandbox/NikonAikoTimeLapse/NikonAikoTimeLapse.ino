
#include <nikonIrControl.h>
#include <AikoEvents.h>

using namespace Aiko;

int CameraIrPin = 8;
int StatusLED = 13;

int frameIntervalSeconds = 5;

void setup()
{
  pinMode(CameraIrPin, OUTPUT);
  pinMode(StatusLED, OUTPUT);
  
  Events.addHandler(takePhoto, frameIntervalSeconds * 1000);
  Events.addHandler(blinkStatusLED, 5000);
}

void takePhoto()
{
  cameraSnap(CameraIrPin);
}

void blinkStatusLED()
{

  digitalWrite(StatusLED, HIGH);
  delay(100);
  digitalWrite(StatusLED, LOW);
}

void loop()
{
  Events.loop();
}

