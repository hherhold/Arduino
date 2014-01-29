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

inline uint32_t pack4bytes( uint8_t* value )
{
    uint32_t retval = 0;
    retval  = (( uint32_t )(value[ 0 ])) << 24;
    retval |= (( uint32_t )(value[ 1 ])) << 16;
    retval |= (( uint32_t )(value[ 2 ])) << 8;
    retval |= value[ 3 ];
    return retval;
}


enum LoggerState
{
    LS_INIT = 0,
    LS_COORDINATOR_FOUND,
    LS_LOGGING
};

#define MAX_SENSOR_NODES 8
XBeeAddress64 sensorNodeList[ MAX_SENSOR_NODES ];
int numSensorNodes = 0;

LoggerState currentLoggerState = LS_INIT;

void setup() 
{
#ifdef USE_DEBUG_CONSOLE
  Bridge.begin();
  Console.begin();
#endif
  xbee.setSerial( mySerial );
  mySerial.begin( 9600 );

  for ( int i = 0; i < MAX_SENSOR_NODES; i++ )
  {
      sensorNodeList[ i ].setMsb( 0 );
      sensorNodeList[ i ].setLsb( 0 );
  }
  
#ifdef USE_DEBUG_CONSOLE
  while (!Console ) { ; }
  
  Console.println( "You're connected to the console!" );
#endif
}

XBeeAddress64 coordinatorAddress64 = XBeeAddress64( 0, 0 );

// During the initialization process, we need synchronized communication,
// meaning a request is sent and a reply is required before proceeding with
// sending another request. This flag is used to denote that a request has been
// sent and a reply is expected.
// Once init is complete, this flag is not used. It's only for setup where
// queries are sent for IDs that need more than one packet to process (64 bit
// IDs, for example).
bool requestInTransit = false;

void handleSending( );

void loop() 
{
    handleSending( );

    xbee.readPacket();
	
    if (xbee.getResponse().isAvailable())
    {
	if ( xbee.getResponse().isError() )
	{
	    Console.print( "Received packet with error status 0x" );
	    Console.println( xbee.getResponse().getErrorCode() );
	}

	requestInTransit = false;
	if ( currentLoggerState == LS_INIT )
	{
	    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
	    {
		AtCommandResponse atCmdResponse;
		xbee.getResponse().getAtCommandResponse( atCmdResponse );

		uint8_t* cmd = atCmdResponse.getCommand();
		uint8_t* value = atCmdResponse.getValue();
		// SH?
		if ( ( cmd[0] == 'S' ) && ( cmd[1] == 'H' ))
		{
		    coordinatorAddress64.setMsb( pack4bytes( value ) );
		}
		else if ( ( cmd[0] == 'S' ) && ( cmd[1] == 'L' ))
		{
		    coordinatorAddress64.setLsb( pack4bytes( value ) );
		    currentLoggerState = LS_LOGGING;
		    Console.println( "Going to LS_LOGGING." );

		    // At this point we no longer need the request in transit flag
		    // as any replies we receive from now on are wholly contained
		    // in one packet. We needed it for the serial number (above) for
		    // the coordinator as the high bytes and low bytes were sent in
		    // two separate packets.
		}
	    }
	}
	else if ( currentLoggerState == LS_LOGGING )
	{
	    if ( xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
	    {
		AtCommandResponse atCommandResponse;
		xbee.getResponse().getAtCommandResponse( atCommandResponse );
		uint8_t* cmd = atCommandResponse.getCommand();
		Console.print( "Received AT_COMMAND_RESPONSE " );
		Console.print( (char)(cmd[0]) );
		Console.println( (char)(cmd[1]) );
	    }
	    else if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE )
	    {
		RemoteAtCommandResponse atCmdResponse;
		xbee.getResponse().getRemoteAtCommandResponse( atCmdResponse );
		Console.print( "Received REMOTE_AT_COMMAND_RESPONSE " );
		uint8_t* cmd = atCmdResponse.getCommand();
		Console.print( (char)(cmd[0]) );
		Console.println( (char)(cmd[1]) );
		Console.print( "Received Remote AT response from:" );
		Console.print( atCmdResponse.getRemoteAddress64().getMsb(), HEX );
		Console.println( atCmdResponse.getRemoteAddress64().getLsb(), HEX );
		if (( cmd[0] == 'N' ) && ( cmd[1] == 'D' ))
		{
		    if ( atCmdResponse.getRemoteAddress64() != coordinatorAddress64 )
		    {
			addAddressToNodeList( atCmdResponse.getRemoteAddress64() );
		    }
		}
	    }
    
	    else if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE)
	    {
		ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
		xbee.getResponse().getZBRxIoSampleResponse(ioSample);
		Console.print("Received I/O Sample from: ");
		Console.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
		Console.println(ioSample.getRemoteAddress64().getLsb(), HEX);  
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
			sendMeasurementToDatabase( radioMac, 
						   MEAS_TYPE_TEMP_C, 
						   temperatureCelsius );
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
    }

    delay( 100 );
}

bool ndSent = false;

void handleSending( )
{
    // Do we need to send any requests?

    // Case 1 - We don't know the coordinator ID. Send out a two-part
    // request for the local radio's serial number. The requestInTransit
    // flag is needed for this two-part request.
    if ( ( currentLoggerState == LS_INIT ) && ( requestInTransit == false ) )
    {
	if ( ( coordinatorAddress64.getMsb() == 0 ) &&
	     ( coordinatorAddress64.getLsb() == 0 ) )
	{
	    // First find coordinator ID. Use OP to get operating ID.
	    Console.println( "Sending out SH request!" );
	    uint8_t cmd[] = {'S','H'};
	    AtCommandRequest request = AtCommandRequest( cmd );
	    xbee.send( request );
	    requestInTransit = true;
	}
	else if ( ( coordinatorAddress64.getMsb() != 0 ) &&
		  ( coordinatorAddress64.getLsb() == 0 ) )
	{
	    // Half of the coordinator ID has been recieved. 
	    // Request the other half.
	    uint8_t cmd[] = {'S','L'};
	    AtCommandRequest request = AtCommandRequest( cmd );
	    xbee.send( request );
	    requestInTransit = true;
	}
    }

    // Normal (logging) operation.
    else if ( currentLoggerState == LS_LOGGING )
    {
	if ( ( numSensorNodes == 0 ) && ( ndSent == false ) )
	{
	    // Do we need to discover the other radios in the network?
	    Console.println( "Sending out ND request!" );
	    // Need to send out ATND command to get all nodes to reply with
	    // their info.
	    uint8_t ndCmd[] = {'N','D'};
	    XBeeAddress64 broadcastAddress = XBeeAddress64( 0x00000000, 0x0000ffff );

	    // The 0, 0 at the end is no value associated with this command.
	    RemoteAtCommandRequest ndRequest = 
		RemoteAtCommandRequest( broadcastAddress, ndCmd, 0, 0 );
	    xbee.send( ndRequest );
	    ndSent = true;
	}
	else
	{
	    Console.print( numSensorNodes );
	}
    }
}

void addAddressToNodeList( XBeeAddress64& newAddress )
{
    for ( int i = 0; i < numSensorNodes; i++ )
    {
	if ( newAddress == sensorNodeList[ i ] )
	{
	    return;
	}
    }
    sensorNodeList[ numSensorNodes ].setMsb( newAddress.getMsb() );
    sensorNodeList[ numSensorNodes ].setLsb( newAddress.getLsb() );
    numSensorNodes++;
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
