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

#define USE_DEBUG_CONSOLE
#ifdef USE_DEBUG_CONSOLE
#include <Console.h>
#include <Bridge.h>
#endif
#include <HttpClient.h>

#include <SoftwareSerial.h>
#include <XBee.h>

// Measurement types. These must match values in the database tables.
#define MEAS_TYPE_TEMP_C		 1
#define MEAS_TYPE_TEMP_F		 2
#define MEAS_TYPE_BAROMETRIC_PRESS_MBAR	 3
#define MEAS_TYPE_HUMIDITY		 4

XBee xbee = XBee();

SoftwareSerial mySerial( 10, 11 ); // rx, tx

void sendMeasurementToDatabase( String radio_mac, int measurementType, float value );


void setup() 
{
#ifdef USE_DEBUG_CONSOLE
  Bridge.begin();
  Console.begin();
#endif
  xbee.setSerial( mySerial );
  mySerial.begin( 9600 );
  
#ifdef USE_DEBUG_CONSOLE
  while (!Console ) { ; }
  
  Console.println( "You're connected to the console!" );
#endif
}

bool discoveryRun = false;

void loop() 
{
    if ( discoveryRun == false ) 
    {
#ifdef USE_DEBUG_CONSOLE
	Console.println( "Sending out ND request!" );
#endif
	// Need to send out ATND command to get all nodes to reply with
	// their info.
	uint8_t ndCmd[] = {'N','D'};
	XBeeAddress64 broadcastAddress = XBeeAddress64( 0x00000000, 0x0000ffff );

	// The 0, 0 at the end is no value associated with this command.
	RemoteAtCommandRequest ndRequest = 
	    RemoteAtCommandRequest( broadcastAddress, ndCmd, 0, 0 );
	xbee.send( ndRequest );
	discoveryRun = true;
    }
    else
    {
	xbee.readPacket();
	
	if (xbee.getResponse().isAvailable())
	{
	    if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE )
	    {
		RemoteAtCommandResponse atCmdResponse;
		xbee.getResponse().getRemoteAtCommandResponse( atCmdResponse );
#ifdef USE_DEBUG_CONSOLE		
		Console.print( "Received Remote AT response from:" );
		Console.print( atCmdResponse.getRemoteAddress64().getMsb(), HEX );
		Console.println( atCmdResponse.getRemoteAddress64().getLsb(), HEX );
#endif
	    }
    
	    else if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE)
	    {
		ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
		xbee.getResponse().getZBRxIoSampleResponse(ioSample);
#ifdef USE_DEBUG_CONSOLE
		Console.print("Received I/O Sample from: ");
		Console.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
		Console.println(ioSample.getRemoteAddress64().getLsb(), HEX);  
#endif
      
		String radioMac = 
		    String( ioSample.getRemoteAddress64().getMsb(), HEX ) +
		    String( ioSample.getRemoteAddress64().getLsb(), HEX );
		radioMac.toUpperCase();
                        
		if (ioSample.containsAnalog()) {
#ifdef USE_DEBUG_CONSOLE
		    Console.println("Sample contains analog data");
		    Console.println( "Grabbing analog sample." );
#endif
		    if ( ioSample.isAnalogEnabled( 0 ) )
		    {
			float temperatureCelsius = 
			    ( ( (ioSample.getAnalog(0) / 1023.0 ) * 1.2 ) - 0.5 ) * 100.0;
			sendMeasurementToDatabase( radioMac, 
						   MEAS_TYPE_TEMP_C, 
						   temperatureCelsius );
		    }
		}
		else
		{
#ifdef USE_DEBUG_CONSOLE
		    Console.println( "Expected analog data in this packet!" );
#endif
		}
	    } 
	    else {
#ifdef USE_DEBUG_CONSOLE
		Console.print("Expected I/O Sample, but got ");
		Console.print(xbee.getResponse().getApiId(), HEX);
#endif
	    }    
	} else if (xbee.getResponse().isError()) {
#ifdef USE_DEBUG_CONSOLE
	    Console.print("Error reading packet.  Error code: ");  
	    Console.println(xbee.getResponse().getErrorCode());
#endif
	}

	delay( 100 );
    }
}

void sendMeasurementToDatabase( String radio_mac, int measurementType, float value )
{
    HttpClient client;
    String url = "http://www.fafoh.com/dbinsert.php?radio_mac=";
    url += radio_mac;
    url += "&type=";
    url += measurementType;
    url += "&value=";
    url += value;
  
    // The result of the get isn't used. Data is being sent to the server to be
    // logged and any response from the server is ignored.
    client.get( url );
}





