// G Force logger

// Use BMA180 accelerometer and Adafruit Logger Shield (with RTC chip) to record g-readings and save them
// to SD card. BMA180 is on i2c bus at address 0x40, and the RTC chip is at 0x68.

#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <BMA180.h>

#define SD_CS_PIN 10

RTC_DS1307 RTC;

BMA180 bma180;


File logfile;


void setup()
{
  Wire.begin( );
  Serial.begin( 19200 );
  
  pinMode( SD_CS_PIN, OUTPUT );
  
  if ( !SD.begin( SD_CS_PIN ) )
  {
    Serial.println( "SD failed to initialize!" );
    for ( ;; ) ;
  }
  Serial.println( "SD initialized." );
  
  Serial.println( "BMA180 Library Testing" );
  Serial.println( "----------------------" );


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
    Serial.println("couldnt create file");
    for ( ;; ) ;
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  int deviceID;
  
  deviceID = bma180.getDeviceID( );
  
  Serial.print( "DeviceID = 0x" );
  Serial.println( deviceID, HEX );
  
  bma180.setRange( BMA180_RANGE_16G );
  
  if ( !RTC.begin( ) )
  {
    Serial.println( "RTC failed" );
  }
}

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  200 // mills between entries (reduce to take more/faster data)
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

void loop()
{
  AccelDataPacket data;
  DateTime now;
  
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");    

  // fetch the time
  now = RTC.now( );
  bma180.getAccelerationData( &data );
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
  
  logfile.print(", ");    
  logfile.print(data.x);
  logfile.print(", ");    
  logfile.print(data.y);
  logfile.print(", ");    
  logfile.print(data.z);

  logfile.println();
 
  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  logfile.flush();
  Serial.println( "Flushed!" );

}

