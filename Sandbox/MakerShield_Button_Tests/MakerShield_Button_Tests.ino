int pbIn = 0;                  // Interrupt 0 is on DIGITAL PIN 2!
int ledOut = 4;                // The output LED pin
volatile int state = LOW;      // The input state toggle
 
void setup()
{               
  // Set up the digital pin 2 to an Interrupt and Pin 4 to an Output
  pinMode(ledOut, OUTPUT);
   
  //Attach the interrupt to the input pin and monitor for ANY Change
  attachInterrupt(pbIn, stateChange, CHANGE);
}
 
void loop()                    
{
  //Simulate a long running process or complex task
  for (int i = 0; i < 100; i++)
  {
    // do nothing but waste some time
    delay(10);
  }
}

void stateChange()
{
  state = !state;
  digitalWrite(ledOut, state); 
}

#if 0

// MakerSheild button tests

volatile int state = LOW;
volatile boolean intPending = false;

void setup()
{
  Serial.begin(115200);
  
  pinMode(8, INPUT);
  pinMode(6, OUTPUT);
  
  attachInterrupt(0, stateChange, CHANGE);
}

void stateChange()
{
  state = !state;
  intPending = true;
}

volatile int buttonState = false;

void loop()
{    
  if (intPending)
  {
    int buttonState = state;
    intPending = false;
  }
  
  if (buttonState)
  {
    Serial.println("Button HIGH");
  }
  else
  {
    Serial.println("Button LOW");
  }
  
  intPending = false;
}



#endif


