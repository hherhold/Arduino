//
// BMA180 Arduino library
//

#include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems

#include <Wire.h>
#include <avr/pgmspace.h>
#include "BMA180.h"


static void writeToBMA180( byte address, byte value )
{
  Wire.beginTransmission( BMA180_DEVICE_ADDR );
  Wire.write( address );
  Wire.write( value );
  Wire.endTransmission( );
}

static int readFromBMA180( byte startAddr, byte numBytesToRead, byte* buf )
{
  Wire.beginTransmission( BMA180_DEVICE_ADDR );
  Wire.write( startAddr );
  Wire.endTransmission( );

  Wire.requestFrom( BMA180_DEVICE_ADDR, numBytesToRead );
  int numBytesAvail = Wire.available( );
  for ( int i = 0; i < numBytesAvail; i++ ) {
    *( buf + i ) = Wire.read( );
  }
  return numBytesAvail;
}

void BMA180::setRange( byte rangeBits )
{
  // Get current value.
  byte lsb1;
  readFromBMA180( BMA180_OFFSET_LSB1_ADDR, 1, &lsb1 );

  // Mask off current bits and or in new ones.
  byte newlsb = lsb1 & !BMA180_RANGE_MASK;
  newlsb |= rangeBits;

  writeToBMA180( BMA180_OFFSET_LSB1_ADDR, newlsb );
}

int BMA180::getDeviceID( )
{
  byte ID;

  int bytesRead = readFromBMA180( BMA180_CHIP_ID_ADDR, 1, &ID );

  return ID;
}

void BMA180::getAccelerationData( AccelDataPacket* accelData )
{
  byte accelRegs[ 7 ];
  readFromBMA180( 0x02, 7, accelRegs );

  accelData->x = accelRegs[ 0 ] >> 2;
  accelData->x |= accelRegs[ 1 ] << 6;
  if ( accelData->x & 0b10000000000000)
  {
    accelData->x |= 0b1110000000000000;
  }
  accelData->y = accelRegs[ 2 ] >> 2;
  accelData->y |= accelRegs[ 3 ] << 6;
  if ( accelData->y &  0b10000000000000)
  {
    accelData->y |= 0b1110000000000000;
  }
  accelData->z = accelRegs[ 4 ] >> 2;
  accelData->z |= accelRegs[ 5 ] << 6;
  if ( accelData->z &  0b10000000000000)
  {
    accelData->z |= 0b1110000000000000;
  }
}

void BMA180::getRawAccelRegs( byte* rawAccelRegs )
{
  byte accelRegs[ 7 ];
  readFromBMA180( 0x02, 7, rawAccelRegs );
}