
// Depository for obsolete code that I want to keep around to look at.

#if 0

void loop() {
  // put your main code here, to run repeatedly:
//  Console.println( "Loop!" );
  //attempt to read a packet    
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    // got something

    if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      Console.print("Received I/O Sample from: ");
      
      Console.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
      Console.print(ioSample.getRemoteAddress64().getLsb(), HEX);  
      Console.println("");
      
      String radioMac = 
          String( ioSample.getRemoteAddress64().getMsb(), HEX ) +
          String( ioSample.getRemoteAddress64().getLsb(), HEX );
      radioMac.toUpperCase();
                        
      Console.print( "full mac = " );
      Console.println( radioMac );
      
      if (ioSample.containsAnalog()) {
        Console.println("Sample contains analog data");
      }

      if (ioSample.containsDigital()) {
        Console.println("Sample contains digtal data");
      }      

      // read analog inputs
      for (int i = 0; i <= 4; i++) {
        if (ioSample.isAnalogEnabled(i)) {
          Console.print("Analog (AI");
          Console.print(i, DEC);
          Console.print(") is ");
          Console.println(ioSample.getAnalog(i), DEC);
        }
      }

      // check digital inputs
      for (int i = 0; i <= 12; i++) {
        if (ioSample.isDigitalEnabled(i)) {
          Console.print("Digital (DI");
          Console.print(i, DEC);
          Console.print(") is ");
          Console.println(ioSample.isDigitalOn(i), DEC);
        }
      }
      
      // method for printing the entire frame data
      //for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
      //  Console.print("byte [");
      //  Console.print(i, DEC);
      //  Console.print("] is ");
      //  Console.println(xbee.getResponse().getFrameData()[i], HEX);
      //}
    } 
    else {
      Console.print("Expected I/O Sample, but got ");
      Console.print(xbee.getResponse().getApiId(), HEX);
    }    
  } else if (xbee.getResponse().isError()) {
    Console.print("Error reading packet.  Error code: ");  
    Console.println(xbee.getResponse().getErrorCode());
  }

#endif

#if 0
  while ( mySerial.available() > 0 )
  {
    Console.println( mySerial.read(), HEX );
  }
#endif
  
