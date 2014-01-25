
// Modified from Rapp's XBee serial test that came with his API 0.9.

// This version uses the Mega's extra serial pins to communicate through the
// XBee. The standard serial port via USB is used for debug info.

#include <XBee.h>

/*
This example is for Series 2 (ZigBee) XBee Radios only
Receives I/O samples from a remote radio.
The remote radio must have IR > 0 and at least one digital or analog input enabled.
The XBee coordinator should be connected to the Arduino.
*/


#define USE_MEGA

#ifdef USE_MEGA
#define XBEE_SERIAL Serial1
#define DEBUG_SERIAL Serial
#else
#define XBEE_SERIAL Serial
#define DEBUG_SERIAL "THIS ONLY WORKS WITH A MEGA RIGHT NOW."
#endif

XBee xbee = XBee();

ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();

XBeeAddress64 test = XBeeAddress64();

void setup()
{ 
  XBEE_SERIAL.begin(9600);
  xbee.setSerial(XBEE_SERIAL);
  DEBUG_SERIAL.begin(9600);
  
  DEBUG_SERIAL.println( "MEGA Debug startup!" );
}

void loop()
{
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
      }

      if (ioSample.containsDigital()) {
        DEBUG_SERIAL.println("Sample contains digtal data");
      }      

      // read analog inputs
      for (int i = 0; i <= 4; i++) {
        if (ioSample.isAnalogEnabled(i)) {
          DEBUG_SERIAL.print("Analog (AI");
          DEBUG_SERIAL.print(i, DEC);
          DEBUG_SERIAL.print(") is ");
          DEBUG_SERIAL.println(ioSample.getAnalog(i), DEC);
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
}
