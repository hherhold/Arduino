
const int ledCount = 6;
const int lastLedIndex = (ledCount - 1);

int ledPins[] = {2, 3, 4, 5, 6, 7};
int currentLed = 0;

void setup()
{
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
   pinMode(ledPins[thisLed], OUTPUT);
  }
}

bool goingForward = true;

void setOnlyOneLed(int ledToSet)
{
  for (int thisLed = 0; thisLed < ledCount; thisLed++)
  {
    if (thisLed == currentLed)
    {
      digitalWrite(ledPins[thisLed], HIGH);
    }
    else
    {
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
}

void loop()
{
  setOnlyOneLed(currentLed);
  delay(100);

  if (goingForward)
  {
    if (currentLed == lastLedIndex)
    {
      goingForward = false;
      currentLed--;
    }
    else
    {
      currentLed++;
    }
  }
  else // going backward
  {
    if (currentLed == 0)
    {
      goingForward = true;
      currentLed++;
    }
    else
    {
      currentLed--;
    }
  }
}

  
