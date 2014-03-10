//
// Yun_WeatherSensors_Sensor.ino
//

#ifdef ARDUINO_AVR_YUN
#include <Console.h>
#include <Bridge.h>
#endif

// Common structures for XBee comm protocol between sensor and base.
// This really doesn't belong in a "library" but the libraries directory
// is kinda the most convenient place for dumping common code.
// (Even though it's only common between the base and sensor in this
// project.)
#include <Yun_WeatherSensors.h>

#include <SoftwareSerial.h>
#include <XBee.h>
#include <SPI.h>
#include <DHT.h>
#include <MPL115A1.h>

// Yes, inconsistent cases. One library I wrote, the other I borrowed.
dht DHT;
MPL115A1 mpl;
#define MPL115A1_ENABLE_PIN 9
#define MPL115A1_SELECT_PIN 7
#define DHT22_PIN 6

// XBee communication. Note that these pins are 
#define ARD_RX_TO_XBEE_TX 4
#define ARD_TX_TO_XBEE_RX 8

SoftwareSerial mySerial( ARD_RX_TO_XBEE_TX, ARD_TX_TO_XBEE_RX ); // rx, tx
XBee xbee = XBee();

#ifdef ARDUINO_AVR_YUN
#define DEBUG_OUT Console
#else
#define DEBUG_OUT Serial
#endif

#define SAMPLE_PERIOD_MILLISECONDS 2000

static unsigned long lastSampleMillis = 0;

void setup()
{
#ifdef ARDUINO_AVR_YUN
    Bridge.begin();

    while ( !Console ) { ; }
    
    Console.println( "You're connected to the console!" );
#else
    // Arduino (pins 0,1) serial is used for downloading and debug,
    // as usual.
    Serial.begin( 9600 );
#endif

    // Software serial is used for XBee.
    xbee.setSerial( mySerial );
    mySerial.begin( 9600 );

    SPI.begin();

    // initialize the chip select and enable pins
    pinMode(MPL115A1_SELECT_PIN, OUTPUT);
    pinMode(MPL115A1_ENABLE_PIN, OUTPUT);
    
    // sleep the MPL115A1
    digitalWrite(MPL115A1_ENABLE_PIN, LOW);

    delay( 1000 );
    
    // wake the MPL115A1
    digitalWrite(MPL115A1_ENABLE_PIN, HIGH);
    delay(20);  // give the chip a few ms to wake up
    
    // set the chip select inactive, select signal is CS LOW
    digitalWrite(MPL115A1_SELECT_PIN, HIGH);

    mpl.init( MPL115A1_SELECT_PIN, MPL115A1_ENABLE_PIN );

    DEBUG_OUT.println( "Start!" );
    lastSampleMillis = millis();
}


void loop()
{
    if ( ( millis() - lastSampleMillis ) > SAMPLE_PERIOD_MILLISECONDS )
    {
        digitalWrite(MPL115A1_ENABLE_PIN, HIGH);
        delay(20);  // give the chip a few ms to wake up
    
        float pressure_pKa = mpl.calcPressure_kPa();

        DEBUG_OUT.print( "MPL115A1: Press (kPa) = " );
        DEBUG_OUT.print( pressure_pKa, 4 );

        // READ DATA
        DEBUG_OUT.print("\tDHT22 ");
        int chk = DHT.read22(DHT22_PIN);
        switch (chk)
        {
        case DHTLIB_OK:  
            DEBUG_OUT.print("read OK,\t"); 
            break;
        case DHTLIB_ERROR_CHECKSUM: 
            DEBUG_OUT.print("Checksum error,\t"); 
            break;
        case DHTLIB_ERROR_TIMEOUT: 
            DEBUG_OUT.print("Time out error,\t"); 
            break;
        default: 
            DEBUG_OUT.print("Unknown error,\t"); 
            break;
        }
        // DISPLAY DATA
        DEBUG_OUT.print( " humidity = " );
        DEBUG_OUT.print(DHT.humidity, 1);
        DEBUG_OUT.print(",\tTemp c = ");
        DEBUG_OUT.println(DHT.temperature, 1);

        lastSampleMillis = millis();
    }

    xbee.readPacket();
    if ( xbee.getResponse().isAvailable() )
    {
        DEBUG_OUT.println( "Got an xbee packet!" );

        DEBUG_OUT.print( "API id = 0x" );
        DEBUG_OUT.println( xbee.getResponse().getApiId(), HEX );

        if ( xbee.getResponse().getApiId() == ZB_RX_RESPONSE )
        {
            ZBRxResponse rxResponse;
            xbee.getResponse().getZBRxResponse( rxResponse );

            DEBUG_OUT.print( "Data length = " );
            DEBUG_OUT.println( rxResponse.getDataLength() );
        }
    }

    DEBUG_OUT.print( "." );
    delay( 100 );
}
