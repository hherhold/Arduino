//
// Proximity Sensor Testing
//

#include <nikonIrControl.h>
#include <AikoEvents.h>

using namespace Aiko;

const int CameraIRPin = 5;

void takePhoto( )
{
  cameraSnap( CameraIRPin );
}

void setup()
{
  Serial.begin( 19200 );
  
  pinMode( CameraIRPin, OUTPUT );

  // OK Really flippin' annoying. The D40 pops out of IR Remote mode every
  // 15 minutes if you don't take a picture with it. So just take one.
  Events.addHandler( takePhoto, (15.0 * 60.0 * 1000 ) );  
}



void loop( )
{
  int val = analogRead( A0 );
  
//  Serial.print( "val = " );
//  Serial.println( val, DEC );
  
  if ( val > 512 )
  {
    cameraSnap( CameraIRPin );
    delay( 900 );
  }
  
  delay( 100 );
}


