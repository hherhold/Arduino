int LED = 11;
int debugLED = 13;

int analogValue = 0;

#define USE_MEGA

#ifdef USE_MEGA
#define XBEE_SERIAL Serial1
#else
#define XBEE_SERIAL Serial
#endif

void setup() {
  pinMode( LED, OUTPUT );
  pinMode( debugLED, OUTPUT );
#ifdef USE_MEGA
  Serial.begin( 9600 );
  Serial.println( "MEGA debug setup()!" );
#endif
  XBEE_SERIAL.begin( 9600 );
}

// Example data:
// 7E 00 14 92 00 13 A2 00 40 89 16 20 FD A2 01 01 00 02 01 00 00 02 B7 5C
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 

void loop() 
{
  if ( XBEE_SERIAL.available() >= 24 )
  {
    // Look for start byte
    if ( XBEE_SERIAL.read() == 0x7E )
    {
      // Blink debug LED when data is received.
      digitalWrite( debugLED, HIGH );
      delay( 10 );
      digitalWrite( debugLED, LOW );
      
      for ( int i = 0; i < 20; i++ )
      {
        byte discard = XBEE_SERIAL.read();
//        Serial.print( discard, HEX );
//        Serial.print( " " );
      }
//      Serial.println( );
      int analogHigh = XBEE_SERIAL.read() & 0xFF;
      int analogLow = XBEE_SERIAL.read() & 0xFF;
      // Change this to shift
      analogValue = analogLow + ( analogHigh * 256 );
    }
    Serial.println( analogValue );
    
    if ( analogValue > 0 && analogValue <= 350 )
    {
      digitalWrite( LED, LOW );
    }
    
    if ( analogValue > 350 && analogValue <= 750 )
    {
      digitalWrite( LED, HIGH );
    }
    
    if ( analogValue > 750 && analogValue <= 1023 )
    {
      digitalWrite( LED, LOW );
    }

  }
}  
  
  
  
  
  
#if 0
  int byteCount = XBEE_SERIAL.available();
  if ( byteCount > 0 )
  {
    Serial.print( "Bytes available = " );
    Serial.println( byteCount );
    byte b = ( XBEE_SERIAL.read() ) & 0xFF;
    if ( b == 0x7E ) Serial.println();
    Serial.print( b, HEX );
    Serial.print( " " );
  }
#endif
#if 0
  // Read until we get a start byte.
  byte b;
  do {
    b = XBEE_SERIAL.read();
  } while ( b != 0x7E );
  Serial.println( "Start byte!" );
  
  byte x0, x1, x2;
  x0 = XBEE_SERIAL.read();
  x1 = XBEE_SERIAL.read();
  x2 = XBEE_SERIAL.read();
  
  Serial.print( x0, HEX ); Serial.print( " " );
  Serial.print( x1, HEX ); Serial.print( " " );
  Serial.print( x2, HEX ); Serial.println( " " );
#endif
  
#if 0
  // Read size and type.
  int packetSize = ( XBEE_SERIAL.read() << 8 ) | XBEE_SERIAL.read();
  byte packetType = XBEE_SERIAL.read();
  
  Serial.print( "Size = " ); Serial.println( packetSize );
  Serial.print( "Type = 0x" ); Serial.print( packetType, HEX );
#endif
#if 0  
  if ( packetType == 0x92 )
  {
#ifdef USE_MEGA
    Serial.println( "Data packet found!" );
    
    byte buffer[ 20 ];
    int i = 0;
    do {
      buffer[ i ] = XBEE_SERIAL.read();
      i++;
    } while ( i < 20 );
    
    Serial.println( "Finished reading data." );
  }
#endif
#endif

#if 0
  if ( XBEE_SERIAL.available() >= 21 )
  {
    // Look for start byte
    if ( XBEE_SERIAL.read() == 0x7E )
    {
      // Blink debug LED when data is received.
      digitalWrite( debugLED, HIGH );
      delay( 10 );
      digitalWrite( debugLED, LOW );
      
      for ( int i = 0; i < 18; i++ )
      {
        byte discard = Serial.read();
        Serial.print( discard, HEX );
      }
      Serial.println();
      int analogHigh = XBEE_SERIAL.read() & 0xFF;
      int analogLow = XBEE_SERIAL.read() & 0xFF;
      // Change this to shift
      analogValue = analogLow | ( analogHigh << 8 );
    }
    
#ifdef USE_MEGA
    Serial.print( "Value = ");
    Serial.println( analogValue );
#endif
    if ( analogValue > 400 )
    {
      digitalWrite( LED, HIGH );
    }
    else
    {
      digitalWrite( LED, LOW );
    }
#endif
#if 0    
    if ( analogValue > 0 && analogValue <= 350 )
    {
      digitalWrite( LED, LOW );
    }
    
    if ( analogValue > 350 && analogValue <= 750 )
    {
      digitalWrite( LED, HIGH );
    }
    
    if ( analogValue > 750 && analogValue <= 1023 )
    {
      digitalWrite( LED, LOW );
    }
#endif

