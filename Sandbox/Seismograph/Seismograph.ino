
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"

#define SD_CS_PIN 10
#define BMA180_INT_PIN 2
#define BMA180_CS_PIN 7

#define RED_LED_PIN 4
#define GREEN_LED_PIN 8

RTC_DS1307 RTC; // define the Real Time Clock object

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


void getAccelData( int* x, int* y, int* z, int* temp )
{
  byte accelRegs[ 7 ];
  readFromBMA180( 0x02, 7, accelRegs ); 
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
}

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(RED_LED_PIN, HIGH);

  while(1);
}

File logfile;

void setup()
{
  Serial.begin( 19200 );
  
  pinMode( RED_LED_PIN, OUTPUT );
  pinMode( GREEN_LED_PIN, OUTPUT );

  // initialize the SD card
  Serial.print( "Initializing SD card..." );
  pinMode( SD_CS_PIN, OUTPUT );

 
  // see if the card is present and can be initialized:
  if ( !SD.begin( SD_CS_PIN ) ) {
    error( "Card failed, or not present" );
  }
  Serial.println( "card initialized." );
  
  // BMA180 Initialization. For some reason, setting these pin
  // modes before initializing the SD card makes the card
  // initialization fail.
  pinMode( BMA180_CS_PIN, OUTPUT );
  pinMode( BMA180_INT_PIN, INPUT );

  byte chipID; 
  readFromBMA180( 0x00, 1, &chipID );
  Serial.print( "Chip ID = " );
  Serial.println( chipID, HEX );
  
  Serial.println( "Setting ee_w to 0" );
  writeTo( 0x0D, 0x0 );
  byte ctrlReg0;
  readFromBMA180( 0x0D, 1, &ctrlReg0 );
  Serial.print( "Reg = " );
  Serial.println( ctrlReg0, HEX );

  Serial.println( "Setting ee_w to 1." );
  writeTo( 0x0D, 0x10 );
  readFromBMA180( 0x0D, 1, &ctrlReg0 );
  Serial.print( "Reg = " );
  Serial.println( ctrlReg0, HEX );

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

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
    Serial.println("RTC failed");
    logfile.println("millis,stamp,datetime,x,y,z");
    logfile.flush();
  }
}

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

void loop()
{
  digitalWrite( GREEN_LED_PIN, HIGH );
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");    

  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
  
  int x, y, z, t;
  
  getAccelData( &x, &y, &z, &t );

  Serial.print( x ); Serial.print( "  " );
  Serial.print( y ); Serial.print( "  " );
  Serial.println( z ); 
  
 
  logfile.print(", ");    
  logfile.print(x);
  logfile.print(", ");    
  logfile.print(y);
  logfile.print(", ");    
  logfile.print(z);

  logfile.println();
  
  digitalWrite( GREEN_LED_PIN, LOW ); 

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  logfile.flush();
}

