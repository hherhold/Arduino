// Possible commands are listed here:
//
// "digital/13"     -> digitalRead(13)
// "digital/13/1"   -> digitalWrite(13, HIGH)
// "analog/2/123"   -> analogWrite(2, 123)
// "analog/2"       -> analogRead(2)
// "mode/13/input"  -> pinMode(13, INPUT)
// "mode/13/output" -> pinMode(13, OUTPUT)


#include <Console.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <SoftwareSerial.h>
#include <XBee.h>

#define USE_DEBUG_CONSOLE

YunServer server;
XBee xbee = XBee();
SoftwareSerial mySerial( 10, 11 ); // rx, tx
XBeeAddress64 coordinatorAddress64 = XBeeAddress64( 0, 0 );
bool initializeCoordinatorAddress( );

#define MAX_SENSOR_NODES 8
XBeeAddress64 sensorNodeList[ MAX_SENSOR_NODES ];
int numSensorNodes = 0;

void checkForXBeePackets( );
void handleNetworkDiscoveryResponse( uint8_t* data, int length );
void addAddressToNodeList( uint32_t serialHigh, uint32_t serialLow );

void setup() 
{
    // Bridge startup
    pinMode(13,OUTPUT);
    digitalWrite(13, LOW);
    Bridge.begin();
    digitalWrite(13, HIGH);

    for ( int i = 0; i < MAX_SENSOR_NODES; i++ )
    {
        sensorNodeList[ i ].setMsb( 0 );
        sensorNodeList[ i ].setLsb( 0 );
    }

    xbee.setSerial( mySerial );
    mySerial.begin( 9600 );

    // Listen for incoming connection only from localhost
    // (no one from the external network could connect)
    server.listenOnLocalhost();
    server.begin();

#ifdef USE_DEBUG_CONSOLE
    while (!Console ) { ; }
  
    Console.println( "You're connected to the console!" );
#endif

#ifdef USE_DEBUG_CONSOLE
    if ( initializeCoordinatorAddress( ) == true )
    {
        Console.println( "Got coordinator address!" );
    }
    else
    {
        Console.println( "Unable to initialize coordinator address!" );
    }
#else
    initializeCoordinatorAddress( );
#endif
}

void loop() 
{
    checkForXBeePackets( );

    YunClient client = server.accept();
    if (client) 
    {
        process(client);
        client.stop();
    }

    delay( 100 );
    Console.print( numSensorNodes );
}

bool initializeCoordinatorAddress( )
{
    sendAtCommand( 'S', 'H' );
    Console.println( "Sent SH" );

    xbee.readPacket( 500 );
    if ( xbee.getResponse().isAvailable() == true )
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

            sendAtCommand( 'S', 'L' );
            Console.println( "Sent SL" );

            xbee.readPacket( 500 );
            if ( xbee.getResponse().isAvailable() == true )
            {
                if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
                {
                    AtCommandResponse atCmdResponse;
                    xbee.getResponse().getAtCommandResponse( atCmdResponse );

                    uint8_t* cmd = atCmdResponse.getCommand();
                    uint8_t* value = atCmdResponse.getValue();
                    // SH?
                    if ( ( cmd[0] == 'S' ) && ( cmd[1] == 'L' ))
                    {
                        coordinatorAddress64.setLsb( pack4bytes( value ) );
                        return true;
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        // Timeout
        return false;
    }
    // Fallthrough.
    return false;
}

void sendAtCommand( uint8_t c1, uint8_t c2 )
{
    uint8_t cmd[] = { c1, c2 };

    AtCommandRequest request = AtCommandRequest( cmd );
    xbee.send( request );
}

void sendRemoteAtCommand( XBeeAddress64& dest, uint8_t c1, uint8_t c2 )
{
    uint8_t cmd[] = { c1, c2 };

    RemoteAtCommandRequest request = 
        RemoteAtCommandRequest( dest, cmd, 0, 0 );
    xbee.send( request );
}


void checkForXBeePackets( )
{
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

        if ( xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
        {
            AtCommandResponse atCommandResponse;
            xbee.getResponse().getAtCommandResponse( atCommandResponse );
            uint8_t* cmd = atCommandResponse.getCommand();
#ifdef USE_DEBUG_CONSOLE
            Console.print( "Received AT_COMMAND_RESPONSE " );
            Console.print( (char)(cmd[0]) );
            Console.println( (char)(cmd[1]) );
#endif

            // Node discovery response?
            if (( cmd[0] == 'N' ) && ( cmd[1] == 'D' ))
            {
                handleNetworkDiscoveryResponse( atCommandResponse.getValue(),
                                                atCommandResponse.getValueLength() );
            }
        }
    }
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

String addressToString( XBeeAddress64& addr )
{
    String s;
    s += String( (uint8_t)( ( addr.getMsb() >> 24 ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getMsb() >> 16 ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getMsb() >>  8 ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getMsb()       ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getLsb() >> 24 ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getLsb() >> 16 ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getLsb() >>  8 ) & 0xFF ), HEX );
    s += ":";
    s += String( (uint8_t)( ( addr.getLsb()       ) & 0xFF ), HEX );
    return s;
}

XBeeAddress64 stringToAddress( String& s )
{
    XBeeAddress64 addr;
    addr.setMsb( 0 );
    addr.setLsb( 0 );

//    strtol( buf1, buf2, 16 );

    return addr;
}


void process(YunClient client) 
{
    // read the command
    String command = client.readStringUntil('/');
    command.trim();

#ifdef USE_DEBUG_CONSOLE
    Console.print( "Got a web request - \'" );
    Console.print( command );
    Console.println( "\'" );
#endif

    if ( command == "coordinator" )
    {
        client.print( coordinatorAddress64.getMsb() );
        client.print( ":" );
        client.print( coordinatorAddress64.getLsb() );
    }

    // Discovery is handled as an "asynchronous" command. XBee routers
    // and end devices can take up to 6-7 seconds to reply to an ND
    // command, so we send it off and then let the main loop() handle
    // responses and adding them to the list of nodes.
    if ( command == "dodiscovery" )
    {
        sendAtCommand( 'N', 'D' );
    }
    if ( command == "listnodes" )
    {
        for ( int i = 0; i < numSensorNodes; i++ )
        {
            client.print( sensorNodeList[ i ].getMsb() );
            client.print( ":" );
            client.print( sensorNodeList[ i ].getLsb() );
            client.print( " " );
        }
    }
    
    if ( command == "measure" )
    {
        uint32_t msb = client.parseInt();
        uint32_t lsb = client.parseInt();
        XBeeAddress64 nodeAddr;
        nodeAddr.setMsb( msb );
        nodeAddr.setLsb( lsb );

        sendRemoteAtCommand( nodeAddr, 'I', 'S' );

        xbee.readPacket( 2000 );
        if ( xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE )
        {
            RemoteAtCommandResponse atCmdResponse;
            xbee.getResponse().getRemoteAtCommandResponse( atCmdResponse );
            uint8_t* cmd = atCmdResponse.getCommand();
            uint8_t status = atCmdResponse.getStatus();
            if ( ( status == 0x0 ) && 
                 (( cmd[0] == 'I' ) && ( cmd[1] == 'S' )) )
            {
                uint8_t* data = atCmdResponse.getValue();
                uint32_t value = ((uint32_t)(data[ 4 ])) << 8;
                value |= ((uint32_t)(data[ 5 ]));
                float temperatureCelsius = 
                    ( ( (value / 1023.0 ) * 1.2 ) - 0.5 ) * 100.0;

                client.print( temperatureCelsius );
            }
        }

//        Console.print( "Node = " );
//        Console.println( addressToString( nodeAddr ) );
    }

#if 0
    // is "digital" command?
    if (command == "digital") {
        digitalCommand(client);
    }

    // is "analog" command?
    if (command == "analog") {
        analogCommand(client);
    }

    // is "mode" command?
    if (command == "mode") {
        modeCommand(client);
    }
#endif
}
#if 0
void digitalCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } 
  else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  }
  else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(YunClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}
#endif
uint32_t pack4bytes( uint8_t* value )
{
    uint32_t retval = 0;
    retval  = (( uint32_t )(value[ 0 ])) << 24;
    retval |= (( uint32_t )(value[ 1 ])) << 16;
    retval |= (( uint32_t )(value[ 2 ])) << 8;
    retval |= value[ 3 ];
    return retval;
}
