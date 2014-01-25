int LED = 11;
int debugLED = 13;

int analogValue = 0;

int remoteIndicator = false;

int lastRemoteIndicator = false;
unsigned long lastSent = 0;

/*
 * Sender config:
 * ATDH, ATDL, ATID as appropriate
 * ATJV1  -> rejoin with coordinator on startup
 * ATD02  -> pin 0 in analog mode with photoresistor
 * ATD14  -> pin 1 in digital output (default low) with LED to ground
 * ATIR64 -> sample rate 100 msec (0x64)
 *
 * Local radio (hooked to arduino or Mac/PC) must be in API mode
 */

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
//    Serial.println( analogValue );
    
    if ( analogValue > 0 && analogValue <= 400 )
    {
      digitalWrite( LED, LOW );
      remoteIndicator = false;
    }
    
    if ( analogValue > 400 && analogValue <= 750 )
    {
      digitalWrite( LED, HIGH );
      remoteIndicator = true;
    }
    
    if ( analogValue > 750 && analogValue <= 1023 )
    {
      digitalWrite( LED, LOW );
      remoteIndicator = false;
    }
    
    if ( ( remoteIndicator != lastRemoteIndicator ) ||
         ( ( millis() - lastSent ) > 10000 ) )
    {
      Serial.println( "State change!" );
      lastRemoteIndicator = remoteIndicator;
      setRemoteState( remoteIndicator ? 0x5 : 0x4 );
      lastSent = millis();
    }      
  }
}  

// 7E 0 10 17 0 0 0 0 0 0 0 FF FF FF FE 2 44 31 4 EB 
// Checksum SB 

// 7E 0 10 17 0 0 0 0 0 0 0 FF FF FF FE 2 44 31 5 EB 
// Checksum SB 

void setRemoteState( int value )
{ 
  byte frameToSend[] = { 0x7E, // Start byte
                         0x0,  // Length MSB - payload minus length byte
                         0x10, // Length LSB (16 bytes)
                         0x17, // Remote AT command
                         0x0,  // Frame ID = 0 (no reply)
                         0x00,
                         0x00,
                         0x00,
                         0x00,
                         0x00,
                         0x00,
                         0xFF, // 0xFF for broadcast
                         0xFF, // 0xFF for broadcast
                         // 16 bit recipient (or 0xFFFE for unknown)
                         0xFF,
                         0xFE,
                         0x02, // Apply changes immediately
                         'D', // Remote command
                         '1', // Remote command
                         value
  };
    
  int checksum = 0;
  int payloadLength = frameToSend[ 2 ];
  int startIndex = 3;
  int endIndex = startIndex + payloadLength;
  for ( int i = startIndex; i < endIndex; i++ )
  {
    checksum += frameToSend[ i ];
  }
  checksum &= 0xFF;
  checksum = 0xFF - checksum;
  
  for ( int i = 0; i < 19; i++ )
  {
    XBEE_SERIAL.write( frameToSend[ i ] ); 
    Serial.print( frameToSend[ i ], HEX );
    Serial.print( " " );

  }
  XBEE_SERIAL.write( checksum );
  Serial.print( checksum, HEX );
  Serial.println( " " );
  delay( 10 );
  
}

