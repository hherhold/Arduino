
// BMA SPI Tests

// BMA180 Pins:
// INT is UNO Pin 2
// CS is UNO Pin 7
// MOSI is UNO Pin 11
// MISO is UNO Pin 12
// SCK is UNO Pin 13

// BMA180 SPI Parameters:
// 1. Bit Order: MSB first, so setBitOrder(MSBFIRST).
// 2. Polarity: polarity = 1, phase = 0 (pp. 58), so SPI.setDataMode(SPI_MODE2) 
//        (see http://arduino.cc/en/Reference/SPI).
// NOTE: For some reason the default settings seem to work fine.

#include <SPI.h>
#include <BMA180.h>

#define BMA180_INT_PIN  2
#define BMA180_CS_PIN   7


void runBMA180SelfTest( )
{ 
  byte statusByte = 0;
  // Get start time, then write st0 to 1.
  int startTime = millis();
  writeTo( BMA180_CTRL_REG0_ADDR, 0x04 );
  int readCount = 0;
  int sampleCount = 0;
  do
  {
    readFromBMA180( BMA180_CTRL_REG0_ADDR, 1, &statusByte );
    
    // Grab data.
    int x, y, z, t;
    if ( getAccelData( &x, &y, &z, &t ) )
    {
      sampleCount++;
    }
    
    readCount++;
  } while ( statusByte & 0x04 );
  int endTime = millis();
  
  Serial.print( "Test time = " );
  Serial.println( endTime - startTime );
  Serial.print( "Reads = " );
  Serial.println( readCount );
  Serial.print( "Sample count = " );
  Serial.println( sampleCount );
    
}


void readFromBMA180( byte startAddress, int numBytesToRead, byte* receiveBuf )
{
  digitalWrite( BMA180_CS_PIN, LOW );
  
  // Bitwise-OR in the R/W bit, bit 8. Datasheet p. 59.
  // 1 = Read, 0 = Write.
  SPI.transfer( startAddress | 0x80 );
  
  for (int i = 0; i < numBytesToRead; i++ )
  {
    receiveBuf[ i ] = SPI.transfer( 0x00 );
  }
  digitalWrite( BMA180_CS_PIN, HIGH );
}

void writeTo( byte registerAddress, byte value )
{
  digitalWrite( BMA180_CS_PIN, LOW );
  
  // Bitwise-AND down the R/W bit, bit 8. Datasheet p. 59.
  // 1 = Read, 0 = Write.
  SPI.transfer( registerAddress & 0xEF );
  SPI.transfer( value );
  
  digitalWrite( BMA180_CS_PIN, HIGH );
}

  
// Some of these are reserved. We just read them all anyway.
byte BMA180RegisterCache[ 0x60 ];  

void dumpRegisterCacheToSerial( )
{
  for ( int i = 0x5F; i > 0x1F; i-- )
  {
    Serial.print( " 0x" ); Serial.print( i, HEX ); Serial.print( "   | " );    // Register Address
    Serial.print( BMA180RegisterCache[ i ], HEX ); Serial.print( "\t| ");  // Value in Hex );
    Serial.println( BMA180RegisterCache[ i ], BIN );
  }   
  for ( int i = 0x10; i > -1; i-- )
  {
    Serial.print( " 0x" ); Serial.print( i, HEX ); Serial.print( "   | " );    // Register Address
    Serial.print( BMA180RegisterCache[ i ], HEX ); Serial.print( "\t| ");  // Value in Hex );
    Serial.println( BMA180RegisterCache[ i ], BIN );
  }
}

// Returns true if new data available, modifying *x, *y, *z, and *temp.
// If new data not available, returns false, leaving pointers unchanged.
boolean getAccelData( int* x, int* y, int* z, int* temp )
{
  byte accelRegs[ 7 ];
  readFromBMA180( 0x02, 7, accelRegs );
  if ( ( accelRegs[ 0 ] & 0x01 ) || ( accelRegs[ 2 ] & 0x01 ) || ( accelRegs[ 3 ] & 0x01 ) )
  {
    *x = accelRegs[ 0 ] >> 2;
    *x |= accelRegs[ 1 ] << 6;
    if ( *x &  0b10000000000000)
    {
      *x |= 0b1110000000000000;
    }
    *y = accelRegs[ 2 ] >> 2;
    *y |= accelRegs[ 3 ] << 6;
    if ( *y &  0b10000000000000)
    {
      *y |= 0b1110000000000000;
    }
    *z = accelRegs[ 4 ] >> 2;
    *z |= accelRegs[ 5 ] << 6;
    if ( *z &  0b10000000000000)
    {
      *z |= 0b1110000000000000;
    }
    *temp = accelRegs[ 6 ];
    
    return true;
  }
  else
  {
    return false;
  }
}

void setup()
{
  Serial.begin( 19200 );
  
// Don't seem to need these?? Odd...  
//  SPI.setClockDivider( SPI_CLOCK_DIV2 );
//  SPI.setBitOrder(MSBFIRST);
//  SPI.setDataMode(SPI_MODE0);

  SPI.begin();
  
  pinMode( BMA180_INT_PIN, INPUT );
  pinMode( BMA180_CS_PIN, OUTPUT );
  
  Serial.println( "Start..." );
  
  byte chipID;
  
  readFromBMA180( 0x00, 1, &chipID );
  Serial.println( chipID, HEX ); 
  
  // Update register cache.
  readFromBMA180( 0x00, 0x60, BMA180RegisterCache );  
  dumpRegisterCacheToSerial( );
  
  // Self test.
  runBMA180SelfTest( );
  
  
  
  // Try setting 
  writeTo( BMA180_CTRL_REG0_ADDR, 0x10 );
  
  byte ctrlReg;
  
  readFromBMA180( BMA180_CTRL_REG0_ADDR, 1, &ctrlReg );
  Serial.print( "Ctrl reg 0 = 0x" );
  Serial.println( ctrlReg, HEX );


  // Set sensitivity to 16g.
  byte xOffsetReg;
  readFromBMA180( 0x35, 1, &xOffsetReg );
  Serial.println( xOffsetReg, HEX );
  Serial.print( "Current offset bits:" );
  Serial.println( ( xOffsetReg >> 1 ) & 0x7, BIN );
  
  xOffsetReg |= 0x0C;
  
  Serial.print( "New val = " );
  Serial.println( xOffsetReg, HEX );
  writeTo( 0x35, xOffsetReg );
  
  readFromBMA180( 0x35, 1, &xOffsetReg );
  Serial.println( xOffsetReg, HEX );
  Serial.print( "Current offset bits:" );
  Serial.println( ( xOffsetReg >> 1 ) & 0x7, BIN );

  Serial.println( "-------------------------" );

  readFromBMA180( 0x00, 0x60, BMA180RegisterCache );  
  dumpRegisterCacheToSerial( );
  
}

void loop()
{
#if 0
  int x, y, z, t;
  
  getAccelData( &x, &y, &z, &t );
  
  Serial.print( x ); Serial.print( "  " );
  Serial.print( y ); Serial.print( "  " );
  Serial.println( z ); 
  
  delay( 100 );
#endif
}

