//  This code generates the IR signals needed to snap the shutter on most Nikon cameras
//  I glued two other bits of information (see comments below) and packaged it for easer use.
//  Any questions or suggestions. email: NikonIrArduinoControl@vonroth.com
//  Webpage for this at: http://www.vonroth.com/Arduino/NikonIrControl
// 11 January 2009 - Gustav von Roth

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// This 39kHz loop from http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino
/* Modulate pin at 39 kHz for give number of microseconds */
void on(int pin, int time) {
  static const int period = 25;
  // found wait_time by measuring with oscilloscope
  static const int wait_time = 9;  

  for (time = time/period; time > 0; time--) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(wait_time);
    digitalWrite(pin, LOW);
    delayMicroseconds(wait_time);
  }
}


void cameraSnap(int pin)
{
// These Timing are from: http://www.bigmike.it/ircontrol/
on(pin,2000);
//This Delay is broken into 3 lines because the delayMicroseconds() is only accurate to 16383. http://arduino.cc/en/Reference/DelayMicroseconds
delayMicroseconds(7830);
delayMicroseconds(10000);
delayMicroseconds(10000);
on(pin,390);
delayMicroseconds(1580);
on(pin,410);
delayMicroseconds(3580);
on(pin,400);
}

