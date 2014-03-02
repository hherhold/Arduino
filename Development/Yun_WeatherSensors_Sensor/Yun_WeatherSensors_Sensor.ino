//
// Yun_WeatherSensors_Sensor.ino
//

#include <SoftwareSerial.h>
#include <XBee.h>
#include <SPI.h>
#include <DHT.h>
#include <MPL115A1.h>

// Yes, inconsistent cases. One library I wrote, the other I borrowed.
dht DHT;
MPL115A1 mpl;

// XBee communication. Note that these pins are 
#define ARD_RX_TO_XBEE_TX 4
#define ARD_TX_TO_XBEE_RX 8

SoftwareSerial mySerial( ARD_RX_TO_XBEE_TX, ARD_TX_TO_XBEE_RX ); // rx, tx
XBee xbee = XBee();

void setup()
{
    // Arduino (pins 0,1) serial is used for downloading and debug,
    // as usual.
    Serial.begin( 9600 );

    // Software serial is used for XBee.
    xbee.setSerial( mySerial );
    mySerial.begin( 9600 );
}

void loop()
{
    xbee.readPacket();
    if ( xbee.getResponse().isAvailable() )
    {
        Serial.println( "Got an xbee packet!" );
    }
}
