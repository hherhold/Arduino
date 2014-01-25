
// Based on code from: http://www.centralnexus.com/seismograph/details_bma180.html

#include <Wire.h>

#include <BMA180.h>


//Writes val to address register on device
void writeTo(byte address, byte val) {
  Wire.beginTransmission(BMA180_DEVICE_ADDR); //start transmission to device 
  Wire.write(address);        // send register address
  Wire.write(val);        // send value to write
  Wire.endTransmission(); //end transmission
}

//reads num bytes starting from address register on device in to buff array
void readFrom(byte address, byte num, byte *buff) {
  Wire.beginTransmission(BMA180_DEVICE_ADDR); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission

  Wire.requestFrom(BMA180_DEVICE_ADDR, num);    // request num bytes from device
  num = Wire.available(); //device may send less than requested (abnormal)
  while(num-- > 0) {
    *(buff++) = Wire.read(); // receive a byte
  }
}

/**
 * Writes val to address register on device if it's different from
 * the current value. This decreases the wear and tear on the EEPROM.
 */
void writeOptionallyTo(byte address, byte val, byte mask) {
  byte value = 0;
  readFrom(address, sizeof(value), &value);
  if ((value & mask) != (val & mask)) {
    // Keep the unmasked values, and changed the masked values.
    writeTo(address, (value & ~mask) | (val & mask));
  }
}

void dumpRegistersToSerial()
{
  byte regValue;
  
  Serial.println( "Addr    |\t|         ");
  for ( byte i = 0x5B; i > 0x3F; i-- )
  {
    readFrom( i, 1, &regValue );
    Serial.print( " 0x" ); Serial.print( i, HEX ); Serial.print( "   | " );    // Register Address
    Serial.print( regValue, HEX ); Serial.print( "\t| ");  // Value in Hex );
    Serial.println( regValue, BIN );
  }
  for ( byte i = 0x3A; i > 0x1F; i-- )
  {
    readFrom( i, 1, &regValue );
    Serial.print( " 0x" ); Serial.print( i, HEX ); Serial.print( "   | " );    // Register Address
    Serial.print( regValue, HEX ); Serial.print( "\t| ");  // Value in Hex );
    Serial.println( regValue, BIN );
  }
  for ( byte i = 0x10; i > 0x00; i-- )
  {
    readFrom( i, 1, &regValue );
    Serial.print( " 0x" ); Serial.print( i, HEX ); Serial.print( "   | " );    // Register Address
    Serial.print( regValue, HEX ); Serial.print( "\t| ");  // Value in Hex );
    Serial.println( regValue, BIN );
  }
}



// Return accelerations in X, Y, and Z axes. 
void getAccelerations( int* xAccel, boolean* newXAvailable,
                       int* yAccel, boolean* newYAvailable,
                       int* zAccel, boolean* newZAvailable,
                       int* temperature )
{
  // For each axis, check for new data and update if available.
  byte tempByte = 0;
  readFrom( BMA180_ACC_X_LSB_ADDR, 1, &tempByte );
  if ( tempByte & BMA180_NEW_ACCEL_DATA_BIT )
  { 
    *newXAvailable = true; 
    *xAccel = 0;
    *xAccel += tempByte >> 2;
    readFrom( BMA180_ACC_X_MSB_ADDR, 1, &tempByte );
    *xAccel += ( tempByte << 6 );
  }
  else
  {
    *newXAvailable = false;
  }
 
  tempByte = 0;
  readFrom( BMA180_ACC_Y_LSB_ADDR, 1, &tempByte );
  if ( tempByte & BMA180_NEW_ACCEL_DATA_BIT )
  {
    *newYAvailable = true;
    *yAccel = 0;
    *yAccel += tempByte >> 2;
    readFrom( BMA180_ACC_Y_MSB_ADDR, 1, &tempByte );
    *yAccel += ( tempByte << 6 );
  }
  else
  {
    *newYAvailable = false;
  }
 
  tempByte = 0;
  readFrom( BMA180_ACC_Z_LSB_ADDR, 1, &tempByte );
  if ( tempByte & BMA180_NEW_ACCEL_DATA_BIT )
  {
    *newZAvailable = true;
    *zAccel = 0;
    *zAccel += tempByte >> 2;
    readFrom( BMA180_ACC_Z_MSB_ADDR, 1, &tempByte );
    *zAccel += ( tempByte << 6 );
  }
  else
  {
    *newZAvailable = false;
  }
  
  // Temperature doesn't have a new data flag.
  tempByte = 0;
  readFrom( BMA180_TEMP_ADDR, 1, &tempByte );
  *temperature = tempByte;
}


void setup()
{
  Wire.begin();          // join i2c bus (address optional for master)
  Serial.begin(19200);  // start serial for output
  Serial.flush();

  // Wait for readings to settle down.
  // 10ms Pause is required to write registers.
  delay(15);

  byte chipID;

  // Enable register write. This must be set 1 to write to any register.
  writeOptionallyTo(BMA180_CTRL_REG0_ADDR, BMA180_CTRL_REG0_EE_W_BIT, BMA180_CTRL_REG0_EE_W_BIT);

  readFrom( BMA180_CHIP_ID_ADDR, 1, &chipID );
  
  Serial.println( "" );
  Serial.print( "Chip ID: 0x" );
  Serial.println( chipID, HEX );
  
  
  
//  writeTo( 0x35, 0x6C ); // 16G
  writeTo( 0x35, 0x64 );  // 2G
  
  dumpRegistersToSerial();
  

  return;
  
//  setOffsets( 0, 0, 0 );
  
#if 0
  int offsetX = 0;
  int offsetY = 0;
  int offsetZ = 0;
  byte offsetTempByte;
  
  readFrom( BMA180_OFFSET_X_MSB_ADDR, 1, &offsetTempByte );
  Serial.print( "Offset X MSB = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetX += offsetTempByte << 4;
  readFrom( BMA180_OFFSET_Y_MSB_ADDR, 1, &offsetTempByte );
  Serial.print( "Offset Y MSB = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetY += offsetTempByte << 4;
  readFrom( BMA180_OFFSET_Z_MSB_ADDR, 1, &offsetTempByte );
  Serial.print( "Offset Z MSB = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetZ += offsetTempByte << 4;
  readFrom( BMA180_OFFSET_LSB1_ADDR, 1, &offsetTempByte );
  Serial.print( "Offset LSB1 Reg = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetX += offsetTempByte >> 4;
  readFrom( BMA180_OFFSET_LSB2_ADDR, 1, &offsetTempByte );
  Serial.print( "Offset LSB2 Reg = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetY += offsetTempByte & 0xF;
  offsetZ += offsetTempByte >> 4;
  Serial.print( "Offset X (dec) = " ); Serial.println( offsetX, DEC );
  Serial.print( "Offset Y (dec) = " ); Serial.println( offsetY, DEC );
  Serial.print( "Offset Z (dec) = " ); Serial.println( offsetZ, DEC );
  
  offsetX = 0;
  offsetY = 0;
  offsetZ = 0;
  offsetTempByte = 0;
  readFrom( BMA180_EE_OFFSET_X_MSB_ADDR, 1, &offsetTempByte );
  Serial.print( "EEOffset X MSB = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetX += offsetTempByte << 4;
  readFrom( BMA180_EE_OFFSET_Y_MSB_ADDR, 1, &offsetTempByte );
  Serial.print( "EEOffset Y MSB = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetY += offsetTempByte << 4;
  readFrom( BMA180_EE_OFFSET_Z_MSB_ADDR, 1, &offsetTempByte );
  Serial.print( "EEOffset Z MSB = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetZ += offsetTempByte << 4;
  readFrom( BMA180_EE_OFFSET_LSB1_ADDR, 1, &offsetTempByte );
  Serial.print( "EEOffset LSB1 Reg = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetX += offsetTempByte >> 4;
  readFrom( BMA180_EE_OFFSET_LSB2_ADDR, 1, &offsetTempByte );
  Serial.print( "EEOffset LSB2 Reg = 0x" ); Serial.println( offsetTempByte, HEX );
  offsetY += offsetTempByte & 0xF;
  offsetZ += offsetTempByte >> 4;
  Serial.print( "EE Offset X (dec) = " ); Serial.println( offsetX, DEC );
  Serial.print( "EE Offset Y (dec) = " ); Serial.println( offsetY, DEC );
  Serial.print( "EE Offset Z (dec) = " ); Serial.println( offsetZ, DEC );
#endif
  
  //
  // SELF TEST
  //
  
  // From Page 48 in datasheet.
  // 1. Measure X, Y, and Z accellerations.
  int x_nom = 0; boolean newXData;
  int y_nom = 0; boolean newYData;
  int z_nom = 0; boolean newZData;
  int temperature;
  
  getAccelerations( &x_nom, &newXData,
                    &y_nom, &newYData,
                    &z_nom, &newZData,
                    &temperature );
                    
  
  int testSampleCount = 30;
  int xTestSamples[ testSampleCount ];
  int yTestSamples[ testSampleCount ];
  int zTestSamples[ testSampleCount ];
  
  boolean done = false;
  int currentSample = 0;
  byte tempByte;
  
  // 2. Initiate self-test.
  unsigned long start = millis();
  writeTo( BMA180_CTRL_REG0_ADDR, 0x04 );
  while ( !done )
  {
    // Read some samples.
    boolean newX, newY, newZ;
    int x, y, z, t;
    getAccelerations( &x, &newX, &y, &newY, &z, &newZ, &t );
    xTestSamples[ currentSample ] = x;
    yTestSamples[ currentSample ] = y;
    zTestSamples[ currentSample ] = z;
    
    currentSample++;
  
    // Check for termination conditions.
    if ( currentSample == 30 )
    {
      done = true;
    }
    readFrom( BMA180_CTRL_REG0_ADDR, 1, &tempByte );
    if ( ( tempByte & 0x4 ) == 0 )
    {
      done = true;
    }
  }
  unsigned long end = millis();

  Serial.print( "Samples recorded during test = " );
  Serial.println( currentSample, DEC );
  Serial.print( "Test duration = " );
  Serial.println( end - start, DEC );
  
  Serial.print( "nom x,y,z = " );
  Serial.print( x_nom, DEC ); Serial.print( ", " );
  Serial.print( y_nom, DEC ); Serial.print( ", " );
  Serial.println( z_nom, DEC );


#if 0

  THIS CODE IS ALL BASED ON GEORGE RHOTEN'S SIESMOGRAPH AND SHOULD NOT BE RUN.
  THIS IS ALL READ-ONLY.
  
  // Enable register write. This must be set 1 to write to any register.
  writeOptionallyTo(BMA180_CTRL_REG0_ADDR, BMA180_CTRL_REG0_EE_W_BIT, BMA180_CTRL_REG0_EE_W_BIT);
  
  // Read some stuff out.
  // I can't figure out how to modify the EEPROM.
  // So we reset upon startup.

  // Change calibration values based on new mode.
  // Each chip will need their own calibration.
  // Note: some offset values affect the spikiness of the values. You should test your values.
  
  // HH
  // mode_config is Section 7.7.3, p. 28 in datasheet. Default setting is 00b and doesn't
  // need to change.
  
  writeOptionallyTo(BMA180_OFFSET_Z_MSB_ADDR, 0x76, 0xFF); // original offset MSB z=0x78 when mode_config=0
  writeOptionallyTo(BMA180_OFFSET_Y_MSB_ADDR, 0x5C, 0xFF); // original offset MSB y=0x60 when mode_config=0
  writeOptionallyTo(BMA180_OFFSET_X_MSB_ADDR, 0x6D, 0xFF); // original offset MSB x=0x70 when mode_config=0
  writeOptionallyTo(BMA180_OFFSET_LSB2_ADDR, 0x7C, 0xFF); // original offset LSB z=0x4, y=0xC when mode_config=0
  // Need a range of existing gravity + 1 G movment to get up to a 9.0M earthquake.
  writeOptionallyTo(BMA180_OFFSET_LSB1_ADDR, 0xC4, 0xFF); // original offset LSB x=0x1, range+12bit=0x4 when mode_config=0

//  writeOptionallyTo(BMA180_TCO_Z_ADDR, 0x01, 0x03); // Change mode_config to lower noise mode
  writeOptionallyTo(BMA180_CTRL_REG3_ADDR, 0x00, BMA180_CTRL_REG3_ADV_INT_BIT); // Turn off adv_int
  
  /* Earthquakes have a nominal frequency range of 0.001–50 Hz. The P and S
   * waves of >2.0M earthquakes usually have a frequency of 0.1-10 Hz. The
   * higher frequencies are attenuated by the bedrock. So you need to be
   * close to the epicenter to measure the higher frequencies. In order to
   * accurately record the lower frequencies, the bandwidth measurement must
   * be lowered in the sensor. When equal to 0x8 or 0x9, the gravity will be
   * cancelled out.
   */
  writeOptionallyTo(BMA180_BW_TCS_ADDR, BMA180_BW_40HZ, BMA180_BW_MASK); // Change bandwidth. 0x2=40Hz 0x3=75Hz Originally 0x4

  writeOptionallyTo(BMA180_CTRL_REG0_ADDR, 0x00, BMA180_CTRL_REG0_EE_W_BIT); // Disable register write for protection

#if USE_LED
  //3 color LED
  pinMode(COMMON_ANODE, OUTPUT);
  digitalWrite(COMMON_ANODE, HIGH);

  pinMode(RED_PIN, OUTPUT);
  digitalWrite(RED_PIN, HIGH);

  pinMode(GREEN_PIN, OUTPUT);
  digitalWrite(GREEN_PIN, HIGH);

  pinMode(BLUE_PIN, OUTPUT);
  digitalWrite(BLUE_PIN, HIGH);
#endif

THIS IS THE END OF GEORGE RHOTEN'S SEISMOGRAPH INIT CODE.

#endif

}

static int last_axis_1 = 0;
static int last_axis_2 = 0;
static int last_axis_3 = 0;

void loop()
{

  byte dataBuf[ 6 ];
  
  readFrom( BMA180_ACC_X_LSB_ADDR, 6, dataBuf );

  Serial.print( dataBuf[ 5 ], HEX ); Serial.print( " " );
  Serial.print( dataBuf[ 4 ], HEX ); Serial.print( " " );
  Serial.print( dataBuf[ 3 ], HEX ); Serial.print( " " );
  Serial.print( dataBuf[ 2 ], HEX ); Serial.print( " " );
  Serial.print( dataBuf[ 1 ], HEX ); Serial.print( " " );
  Serial.println( dataBuf[ 0 ], HEX );
#if 0
  int xAccel = ( dataBuf[ 1 ] << 6 ) | ( dataBuf[ 0 ] >> 2 );
  if ( xAccel &  0b10000000000000)
  {
    xAccel |= 0b1110000000000000;
  }

  int yAccel = ( dataBuf[ 3 ] << 6 ) | ( dataBuf[ 2 ] >> 2 );
  if ( yAccel &  0b10000000000000)
  {
    yAccel |= 0b1110000000000000;
  }

  int zAccel = ( dataBuf[ 5 ] << 6 ) | ( dataBuf[ 4 ] >> 2 );
  if ( zAccel &  0b10000000000000)
  {
    zAccel |= 0b1110000000000000;
  }

  Serial.print( xAccel, DEC ); Serial.print( "\t" );
  Serial.print( yAccel, DEC ); Serial.print( "\t" );
  Serial.println( zAccel, DEC );
#endif
  
  delay(1000);
  
  
  
#if 0
  // 2 byte endian marker
  // 6 byte buffer for saving data read from the device
  // 2 byte checksum in case there is a reset in the middle of a packet.
  int axis[5] = {0x8081, 0, 0, 0, 0};

  // There are 1,000,000 microseconds per second,
  // and we want to sample about 200 per second.
  // This gives us about the right rate with the rest of the overhead.
  delayMicroseconds(DELAY_RATE - (int)(micros() % DELAY_RATE));
  
  // Each axis reading comes in 14 bit resolution (2 bytes little endian).
  readFrom(DATA_X0, 6, (byte*)(axis+1)); //read the acceleration data

  // Remove status and 0 bits
  axis[1] = axis[1] >> AXIS_SHIFT;
  axis[2] = axis[2] >> AXIS_SHIFT;
  axis[3] = axis[3] >> AXIS_SHIFT;

  // Calculate checksum.
//  axis[4] = axis[1] + axis[2] + axis[3];
  // Write whole packet.
//  Serial.write((byte *)axis, sizeof(axis));

// Delta detection
  if ( (axis[1] != last_axis_1) || (axis[2] != last_axis_2) || (axis[3] != last_axis_3))
  {
    last_axis_1 = axis[1];
    last_axis_2 = axis[2];
    last_axis_3 = axis[3];

    Serial.print( axis[1], DEC );
    Serial.print( "  " );
    Serial.print( axis[2], DEC );
    Serial.print( " " );
    Serial.println( axis[3], DEC );
  }
  
#endif
}
