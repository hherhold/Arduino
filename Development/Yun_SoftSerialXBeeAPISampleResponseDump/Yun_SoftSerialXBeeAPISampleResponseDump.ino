#include <Console.h>
#include <SoftwareSerial.h>

#include <XBee.h>

XBee xbee = XBee();

ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();

XBeeAddress64 test = XBeeAddress64();


SoftwareSerial mySerial( 10, 11 ); // rx, tx

void setup() {
  // put your setup code here, to run once:
  Bridge.begin();
  Console.begin();
  xbee.setSerial( mySerial );
  mySerial.begin( 9600 );
  
  while (!Console ) { ; }
  
  Console.println( "You're connected to the console!" );
}

#define DEBUG_SERIAL Console

void loop() {
  // put your main code here, to run repeatedly:
//  Console.println( "Loop!" );
  //attempt to read a packet    
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    // got something

    if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      DEBUG_SERIAL.print("Received I/O Sample from: ");
      
      DEBUG_SERIAL.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
      DEBUG_SERIAL.print(ioSample.getRemoteAddress64().getLsb(), HEX);  
      DEBUG_SERIAL.println("");
      
      if (ioSample.containsAnalog()) {
        DEBUG_SERIAL.println("Sample contains analog data");
        byte analogMask = ioSample.getAnalogMask();
        DEBUG_SERIAL.print( "Analog mask: 0x" );
        DEBUG_SERIAL.println( analogMask, HEX );
      }

      if (ioSample.containsDigital()) {
        DEBUG_SERIAL.println("Sample contains digtal data");
      }      

      // read analog inputs
      for (int i = 0; i <= 4; i++) {
        if (ioSample.isAnalogEnabled(i)) {
          DEBUG_SERIAL.print("Analog (AI");
          DEBUG_SERIAL.print(i, DEC);
          DEBUG_SERIAL.print(") is 0x");
          DEBUG_SERIAL.print(ioSample.getAnalog(i), HEX);
          DEBUG_SERIAL.print(" (");
          DEBUG_SERIAL.print(ioSample.getAnalog(i), DEC);
          DEBUG_SERIAL.print("d) (");
          DEBUG_SERIAL.print(ioSample.getAnalog(i) * 1200.0 / 1023.0 );
          DEBUG_SERIAL.println(" mV)");
          
          float temperatureCelsius = 
            ( ( (ioSample.getAnalog(i) / 1023.0 ) * 1.2 ) - 0.5 ) * 100.0;
          DEBUG_SERIAL.print( "Temp = " );
          DEBUG_SERIAL.println( temperatureCelsius );
        }
      }

      // check digital inputs
      for (int i = 0; i <= 12; i++) {
        if (ioSample.isDigitalEnabled(i)) {
          DEBUG_SERIAL.print("Digital (DI");
          DEBUG_SERIAL.print(i, DEC);
          DEBUG_SERIAL.print(") is ");
          DEBUG_SERIAL.println(ioSample.isDigitalOn(i), DEC);
        }
      }
      
      // method for printing the entire frame data
      //for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
      //  DEBUG_SERIAL.print("byte [");
      //  DEBUG_SERIAL.print(i, DEC);
      //  DEBUG_SERIAL.print("] is ");
      //  DEBUG_SERIAL.println(xbee.getResponse().getFrameData()[i], HEX);
      //}
    } 
    else {
      DEBUG_SERIAL.print("Expected I/O Sample, but got ");
      DEBUG_SERIAL.print(xbee.getResponse().getApiId(), HEX);
    }    
  } else if (xbee.getResponse().isError()) {
    DEBUG_SERIAL.print("Error reading packet.  Error code: ");  
    DEBUG_SERIAL.println(xbee.getResponse().getErrorCode());
  }



#if 0  
  while ( mySerial.available() > 0 )
  {
    Console.println( mySerial.read(), HEX );
  }
#endif
  
//  delay( 200 );
}
