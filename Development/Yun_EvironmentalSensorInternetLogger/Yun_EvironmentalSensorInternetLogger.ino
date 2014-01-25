/*
 * Environmental Sensor Internet Logger
 * 
 * Grabs environmental sensor data (temperature, relative humidity, barometric
 * pressure) from multiple locations using XBee radios.
 *
 * This is very much a work in progress - a "living application", as it were.
 *
 * The first implementation uses only temperature sensors as they can be connected
 * directly to an XBee module and data grabbed from an IO pin. More complicated
 * sensors that use I2C or SPI will require use of a PIC like Arduino.
 */

/*
 * To do:
 * 1. Send data to fafoh.com via HTML request over Bridge interface.
 * 2. Figure out how to tell XBee sensors to sleep inbetween reads.
 * 3. Determine sample period. 1 minute for starters.
 */
 
#include <Console.h>
#include <Bridge.h>
#include <HttpClient.h>

#include <SoftwareSerial.h>
#include <XBee.h>

XBee xbee = XBee();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();

SoftwareSerial mySerial( 10, 11 ); // rx, tx

void sendMeasurementToDatabase( String radio_mac, int measurementType, float value );

#define USE_DEBUG_CONSOLE

HttpClient myClient;

void setup() {
  // put your setup code here, to run once:
  Bridge.begin();
  Console.begin();
  xbee.setSerial( mySerial );
  mySerial.begin( 9600 );
  
#ifdef USE_DEBUG_CONSOLE
  while (!Console ) { ; }
#endif
  
  Console.println( "You're connected to the console!" );
}

// Only send every 5 samples.
int decimationCounter = 5;

void loop() {
  //attempt to read a packet    
  xbee.readPacket();

  if (xbee.getResponse().isAvailable())
  {
    // got something
    
    if (xbee.getResponse().getApiId() == AT_RESPONSE )
    {
      RemoteAtCommandResponse atCmdResponse;
      xbee.getResponse().getRemoteAtCommandResponse( atCmdResponse );
      
      Console.print( "Received AT response from:" );
      String responseMac = 
        String( atCmdResponse.getRemoteAddress64().getMsb(), HEX ) +
        String( atCmdResponse.getRemoteAddress64().getLsb(), HEX );
      responseMac.toUpperCase();
      Console.println( responseMac );
    }
    
    else if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE)
    {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      Console.print("Received I/O Sample from: ");
      
      Console.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
      Console.print(ioSample.getRemoteAddress64().getLsb(), HEX);  
      Console.println("");
      
      String radioMac = 
          String( ioSample.getRemoteAddress64().getMsb(), HEX ) +
          String( ioSample.getRemoteAddress64().getLsb(), HEX );
      radioMac.toUpperCase();
                        
                        
      if (ioSample.containsAnalog()) {
        Console.println("Sample contains analog data");
        Console.println( "Grabbing analog sample." );
        if ( ioSample.isAnalogEnabled( 0 ) )
        {
          float temperatureCelsius = 
            ( ( (ioSample.getAnalog(0) / 1023.0 ) * 1.2 ) - 0.5 ) * 100.0;
          
          decimationCounter--;
          if ( decimationCounter <= 0 )
          {
            Console.println( "Sending to DB!" );
            sendMeasurementToDatabase( radioMac, 1, temperatureCelsius );
            decimationCounter = 5;
          }
       }
      }
      else
      {
        Console.println( "Expected analog data in this packet!" );
      }
    } 
    else {
      Console.print("Expected I/O Sample, but got ");
      Console.print(xbee.getResponse().getApiId(), HEX);
    }    
  } else if (xbee.getResponse().isError()) {
    Console.print("Error reading packet.  Error code: ");  
    Console.println(xbee.getResponse().getErrorCode());
  }



#if 0  
  while ( mySerial.available() > 0 )
  {
    Console.println( mySerial.read(), HEX );
  }
#endif
  
//  delay( 200 );
}

// Format: http://www.fafoh.com/dbinsert.php?location=1&type=1&value=70.0

void sendMeasurementToDatabase( String radio_mac, int measurementType, float value )
{
  String url = "http://www.fafoh.com/dbinsert.php?radio_mac=";
  url += radio_mac;
  url += "&type=";
  url += measurementType;
  url += "&value=";
  url += value;
  
  // The result of the get isn't used. Data is being sent to the server to be
  // logged and any response from the server is ignored.
  myClient.get( url );
  
}





