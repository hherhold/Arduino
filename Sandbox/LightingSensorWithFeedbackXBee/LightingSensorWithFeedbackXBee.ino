
#define VERSION "1.02"

int LED = 11;
int debugLED = 13;
int analogValue = 0;
int remoteIndicator = false;


int lastRemoteIndicator = false;
int lastSent = 0;

void setup( )
{
  pinMode( LED, OUTPUT );
  pinMode( debugLED, OUTPUT );
  Serial.begin( 9600 );
  Serial1.begin( 9600 );
}

void loop()
{
  if (Serial1.available() >= 23 )
  {
    // Look for start byte
    if ( Serial1.read() == 0x7E )
    {
      // Blink debug LED to indicate when data is received
      digitalWrite( debugLED, HIGH );
      delay( 10 );
      digitalWrite( debugLED, LOW );
      
      // Read the variables we're not using out of the buffer
      // (included two more for the digtal pin report)
      for ( int i = 0; i < 20; i++ )
      {
        byte discard = Serial1.read();
      }
      int analogHigh = Serial1.read();
      int analogLow = Serial1.read();
      analogValue = analogLow + ( analogHigh * 256 );
    }
  }
  
//  Serial.println( analogValue );
  // Adjust accordingly
  if ( analogValue > 0 && analogValue <= 350 )
  {
//    Serial.println( "LOW!" );
    digitalWrite( LED, LOW );
    remoteIndicator = false;
  }
  
  if ( analogValue > 350 && analogValue <= 750 )
  {
//    Serial.println( "HIGH!" );
    digitalWrite( LED, HIGH );
    remoteIndicator = true;
  }
#if 0  
  if ( analogValue > 750 && analogValue <= 1023 )
  {
//    Serial.println( "LOW!" );
    digitalWrite( LED, LOW );
    remoteIndicator = false;
  }
#endif
  // Set the indicator immediately when there's a state change.
  if ( remoteIndicator != lastRemoteIndicator )
  {
    if ( remoteIndicator == false )
    {
      setRemoteState( 0x4 );
    }
    if ( remoteIndicator == true )
    {
      setRemoteState( 0x5 );
    }
    
    lastRemoteIndicator = remoteIndicator;
  }
#if 0  
  // Re-set periodically.
  if ( millis() - lastSent > 10000 )
  {
    if ( remoteIndicator == false )
    {
      setRemoteState( 0x4 );
    }
    if ( remoteIndicator == true )
    {
      setRemoteState( 0x5 );
    }
    
    lastSent = millis();
  }
#endif
}

void setRemoteState( int value )
{
  
  Serial.print( "setRemoteState( " );
  Serial.print( value );
  Serial.println( ")" );
#if 0
  int bufSize = 18;
  
  byte buffer[  ] = 
                  { 0x7E, 0x00, 0x10, 0x17, 0x00,          // 0-4
                    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,    // 5-10
                    0xFF, 0xFE,                            // 11-12
                    0x02,                                  // 13
                    'D', '1',                              // 14-15
                    value,                                 // 16
                    0x00 };                                // 17
  int checksum = 0;
  for ( int i = 3; i < bufSize; i++ )
  {
    checksum += buffer[ i ];
  }
  
  buffer[ bufSize - 1 ] = 0xFF - ( checksum & 0xFF );
  
  Serial1.write( buffer, 18 );
  
  delay( 10 );
#endif
#if 1
  Serial1.write( (byte)0x7E );  // Start byte
  Serial1.write( (byte)0x00 );  // High part of length (always zero)
  Serial1.write( (byte)0x10 );  // Low part of length (num bytes, not including checksum)
  Serial1.write( (byte)0x17 );  // 0x17 = remote AT
  Serial1.write( (byte)0x00 );  // Frame ID set to zero for no reply
  
  // ID of recipient, or use 0xFFFF for broadcast
  Serial1.write( (byte)0x00 );
  Serial1.write( (byte)0x00 );
  Serial1.write( (byte)0x00 );
  Serial1.write( (byte)0x00 );
  Serial1.write( (byte)0x00 );
  Serial1.write( (byte)0x00 );
  Serial1.write( (byte)0xFF );
  Serial1.write( (byte)0xFF );

  // 16 bit of recipient or 0xFFFE if unknown.
  Serial1.write( (byte)0xFF );
  Serial1.write( (byte)0xFE );
  
  Serial1.write( (byte)0x02 ); // 0x02 to apply canges immediately on rmeote
  
  // Command name in ASCII
  Serial1.write( 'D' );
  Serial1.write( '1' );
  
  // Command data in as many bytes as needed
  Serial1.write( value );
  
  // Checksum.
  long sum = 0x17 + 0xFF + 0xFF + 0xFF + 0xFE + 0x02 + 'D' + '1' + value;
  Serial1.write( (byte)( 0xFF - ( sum & 0xFF ) ) );
  delay( 10 );
#endif
}

  

