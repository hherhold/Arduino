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

#ifdef ARDUINO_AVR_YUN

#endif

#ifdef ARDUINO_AVR_MEGA2560
#define FOO 2
#endif

#undef USE_DEBUG_CONSOLE

#ifdef USE_DEBUG_CONSOLE
#include <Console.h>
#endif

#include <Bridge.h>
#include <Process.h>

// #include <YunServer.h>
// #include <YunClient.h>

#include <SoftwareSerial.h>
#include <XBee.h>

// Measurement types. These must match values in the database tables.
#define MEAS_TYPE_TEMP_C		 1
#define MEAS_TYPE_TEMP_F		 2
#define MEAS_TYPE_BAROMETRIC_PRESS_MBAR	 3
#define MEAS_TYPE_HUMIDITY		 4

XBee xbee = XBee();
SoftwareSerial mySerial( 10, 11 ); // rx, tx

uint32_t pack4bytes( uint8_t* value )
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

XBeeAddress64 coordinatorAddress64 = XBeeAddress64( 0, 0 );

// YunServer server;

// During the initialization process, we need synchronized communication,
// meaning a request is sent and a reply is required before proceeding with
// sending another request. This flag is used to denote that a request has been
// sent and a reply is expected.
// Once init is complete, this flag is not used. It's only for setup where
// queries are sent for IDs that need more than one packet to process (64 bit
// IDs, for example).
// Currently the only request this is used for is to determine the coordinator
// radio's 64-bit address, using the SH and SL commands separately.
bool requestInTransit = false;

void handleSending( );
void handleNetworkDiscoveryResponse( uint8_t* data, int length );
void addAddressToNodeList( uint32_t serialHigh, uint32_t serialLow );
void sendMeasurementToDatabase( String radio_mac, int measurementType, float value );

// void processServerRequest( YunClient client );

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
  
  // Yun server to receive commands from Linino.
//  server.listenOnLocalhost();
//  server.begin();

#ifdef USE_DEBUG_CONSOLE
  while (!Console ) { ; }
  
  Console.println( "You're connected to the console!" );
#endif
}

void loop() 
{
//    YunClient client = server.accept();
//
//    if ( client )
//    {
//        processServerRequest( client );
//        client.stop();
//    }

    handleSending( );

    xbee.readPacket();
	
    if (xbee.getResponse().isAvailable())
    {
	if ( xbee.getResponse().isError() )
	{
#ifdef USE_DEBUG_CONSOLE
	    Console.print( "Received packet with error status 0x" );
	    Console.println( xbee.getResponse().getErrorCode() );
#endif
	}
#ifdef USE_DEBUG_CONSOLE
	Console.print( "Packet available. API ID = 0x" );
	Console.println( xbee.getResponse().getApiId(), HEX );
#endif

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
#if 0
		    Console.print( "Coordinator address: " );
		    String s = 
			String( coordinatorAddress64.getMsb(), HEX ) +
			String( coordinatorAddress64.getLsb(), HEX );
		    Console.println( s );
		    Console.println( "Going to LS_LOGGING." );
#endif
		    currentLoggerState = LS_LOGGING;

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
#ifdef USE_DEBUG_CONSOLE
		Console.print( "Received AT_COMMAND_RESPONSE " );
		Console.print( (char)(cmd[0]) );
#endif		Console.println( (char)(cmd[1]) );

		// Node discovery response?
		if (( cmd[0] == 'N' ) && ( cmd[1] == 'D' ))
		{
		    handleNetworkDiscoveryResponse( atCommandResponse.getValue(),
						    atCommandResponse.getValueLength() );
		}
	    }
	    else if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE )
	    {
		RemoteAtCommandResponse atCmdResponse;
		xbee.getResponse().getRemoteAtCommandResponse( atCmdResponse );
		uint8_t* cmd = atCmdResponse.getCommand();
                uint8_t status = atCmdResponse.getStatus();
		String radioMac = 
		    String( atCmdResponse.getRemoteAddress64().getMsb(), HEX ) +
		    String( atCmdResponse.getRemoteAddress64().getLsb(), HEX );
		radioMac.toUpperCase();

#ifdef USE_DEBUG_CONSOLE
		Console.print( "Received REMOTE_AT_COMMAND_RESPONSE " );
		Console.print( (char)(cmd[0]) );
		Console.println( (char)(cmd[1]) );
                Console.print( "Status = 0x" );
                Console.println( status, HEX );
		Console.print( "Received Remote AT response from:" );
		Console.println( radioMac );
#endif
                if ( ( status == 0x0 ) && 
                     (( cmd[0] == 'I' ) && ( cmd[1] == 'S' )) )
		{
		    uint8_t* data = atCmdResponse.getValue();
		    uint32_t value = ((uint32_t)(data[ 4 ])) << 8;
		    value |= ((uint32_t)(data[ 5 ]));
		    float temperatureCelsius = 
			( ( (value / 1023.0 ) * 1.2 ) - 0.5 ) * 100.0;
#ifdef USE_DEBUG_CONSOLE
		    Console.print( "Logging temperature: " );
		    Console.println( temperatureCelsius );
#endif
		    sendMeasurementToDatabase( radioMac, 
					       MEAS_TYPE_TEMP_C, 
					       temperatureCelsius );

		}
	    }
#if 0    
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
			Console.print( "Logging temperature: " );
			Console.println( temperatureCelsius );
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
	    else 
            {
		Console.print("Expected I/O Sample, but got ");
		Console.print(xbee.getResponse().getApiId(), HEX);
	    }
#endif    
	} 
        else if (xbee.getResponse().isError()) 
        {
#ifdef USE_DEBUG_CONSOLE
	    Console.print("Error reading packet.  Error code: ");  
	    Console.println(xbee.getResponse().getErrorCode());
#endif
	}
    }

    delay( 100 );
}

void handleNetworkDiscoveryResponse( uint8_t* data, int length )
{
#if 0
    Console.print( "ND payload: " );
    for ( int i = 0; i < length; i++ )
    {
	Console.print( (char)(data[i]), HEX );
	Console.print( " " );
    }
    Console.println( "" );
#endif

    uint32_t serialHigh = pack4bytes( data + 2 );
    uint32_t serialLow  = pack4bytes( data + 6 );

    addAddressToNodeList( serialHigh, serialLow );
}

bool ndSent = false;

unsigned int samplePeriodInSeconds = 10;
unsigned long delayTimerMillis = 0;

void sendNetworkDiscoveryRequest( )
{
    // Do we need to discover the other radios in the network?
#ifdef USE_DEBUG_CONSOLE
    Console.println( "Sending out ND request!" );
#endif
    // Need to send out ATND command to get all nodes to reply with
    // their info.
    uint8_t ndCmd[] = {'N','D'};

    AtCommandRequest ndRequest = AtCommandRequest( ndCmd );
    xbee.send( ndRequest );
}


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
#ifdef USE_DEBUG_CONSOLE
	    Console.println( "Sending out SH request!" );
#endif
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
            sendNetworkDiscoveryRequest( );
	    ndSent = true;
	}
	else
	{
	    // Logging "steady-state".
	    if ( ( millis() - delayTimerMillis ) > ( samplePeriodInSeconds * 1000 ) )
	    {
		// Time to request sensor data.
		for ( int i = 0; i < numSensorNodes; i++ )
		{
		    uint8_t cmd[] = {'I','S'};

		    // The 0, 0 at the end is no value associated with this command.
		    RemoteAtCommandRequest request = 
			RemoteAtCommandRequest( sensorNodeList[ i ], cmd, 0, 0 );
		    xbee.send( request );
		}
		delayTimerMillis = millis();
#ifdef USE_DEBUG_CONSOLE
		Console.print( "X" );
#endif
		if ( numSensorNodes == 0 )
		{
		    ndSent = false;
		}
	    }
#ifdef USE_DEBUG_CONSOLE
	    Console.print( numSensorNodes );
#endif
	}
    }
}

void addAddressToNodeList( uint32_t serialHigh, uint32_t serialLow )
{
    // Verify that this is a good address and got garbage (all zeros).
    if ( ( serialHigh == 0 ) || ( serialLow == 0 ))
    {
#ifdef USE_DEBUG_CONSOLE
	Console.println( "IGNORING add of bogus address to node list!" );
#endif
	return;
    }
    if ( ( serialHigh == coordinatorAddress64.getMsb() ) &&
	 ( serialLow  == coordinatorAddress64.getLsb() ) )
    {
	return;
    }
    for ( int i = 0; i < numSensorNodes; i++ )
    {
	if ( ( serialHigh == sensorNodeList[ i ].getMsb() ) &&
	     ( serialLow  == sensorNodeList[ i ].getLsb() ) )
	{
	    return;
	}
    }
    sensorNodeList[ numSensorNodes ].setMsb( serialHigh );
    sensorNodeList[ numSensorNodes ].setLsb( serialLow );
    numSensorNodes++;
}

void sendMeasurementToDatabase( String radio_mac, int measurementType, float value )
{
    Process client;

    String command = "curl -A \"Mozilla\" \"http://www.fafoh.com/dbinsert.php?radio_mac=";
    command += radio_mac;
    command += "&type=";
    command += measurementType;
    command += "&value=";
    command += value;
    command += "\"";

//    Console.println( command );
    client.runShellCommand( command );

#if 0
    client.begin( "curl" );
    client.addParameter( "-A \"Mozilla\"" );

    String url = "\"http://www.fafoh.com/dbinsert.php?radio_mac=";
    url += radio_mac;
    url += "&type=";
    url += measurementType;
    url += "&value=";
    url += value;
    url += "\"";

    Console.println( url );

    client.run();
#endif

#if 0
    HttpClient client;
    String url = "-A \"Mozilla\" \"http://www.fafoh.com/dbinsert.php?radio_mac=";
    url += radio_mac;
    url += "&type=";
    url += measurementType;
    url += "&value=";
    url += value;
    url += "\"";

    // The result of the get isn't used. Data is being sent to the server to be
    // logged and any response from the server is ignored.
    client.get( url );

    Console.println( url );

    //    Console.println( "SENDING TO DB DISBALED FOR NOW!" );
#endif
}

// void processServerRequest( YunClient client )
// {
//     String command = client.readStringUntil( '/' );
//     if ( command == "samplePeriod" )
//     {
//         int value = client.parseInt();
//         samplePeriodInSeconds = value;
// #ifdef USE_DEBUG_CONSOLE
//         Console.print( "NEW delay set to " );
//         Console.print( samplePeriodInSeconds );
//         Console.println( " seconds." );
// #endif
//     }
// }

