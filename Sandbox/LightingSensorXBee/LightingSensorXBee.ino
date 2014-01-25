
#define VERSION "1.02"

int LED = 11;
int debugLED = 13;
int analogValue = 0;

void setup( )
{
  pinMode( LED, OUTPUT );
  pinMode( debugLED, OUTPUT );
  Serial.begin( 9600 );
  Serial1.begin( 9600 );
}

void loop()
{
  if (Serial1.available() >= 21 )
  {
    if ( Serial1.read() == 0x7E )
    {
      digitalWrite( debugLED, HIGH );
      delay( 10 );
      digitalWrite( debugLED, LOW );
      
      for ( int i = 0; i < 18; i++ )
      {
        byte discard = Serial1.read();
      }
      int analogHigh = Serial1.read();
      int analogLow = Serial1.read();
      analogValue = analogLow + ( analogHigh * 256 );
    }
  }
  
  if ( analogValue > 0 && analogValue <= 350 )
  {
    Serial.println( "LOW!" );
    digitalWrite( LED, LOW );
  }
  
  if ( analogValue > 350 && analogValue <= 750 )
  {
    Serial.println( "HIGH!" );
    digitalWrite( LED, HIGH );
  }
  
  if ( analogValue > 750 && analogValue <= 1023 )
  {
    Serial.println( "LOW!" );
    digitalWrite( LED, LOW );
  }
}

