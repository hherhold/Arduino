//
// Testing of the AVAGO HDJD-S822 Color Sensor.
//

#define ANALOG_RED_PIN   2
#define ANALOG_GREEN_PIN 1
#define ANALOG_BLUE_PIN  0

#define LED_PIN A7

void setup()
{
  Serial.begin( 19200 );
  
  pinMode( LED_PIN, OUTPUT );
  digitalWrite( LED_PIN, HIGH );
}

void loop( )
{
  int redValue   = analogRead( ANALOG_RED_PIN );
  int greenValue = analogRead( ANALOG_GREEN_PIN );
  int blueValue = analogRead( ANALOG_BLUE_PIN );
  
  Serial.print( "R = " ); Serial.print( redValue ); Serial.print("  " );
  Serial.print( "G = " ); Serial.print( greenValue ); Serial.print("  " );
  Serial.print( "B = " ) ;Serial.println( blueValue );
  
  delay( 1000 );
}

