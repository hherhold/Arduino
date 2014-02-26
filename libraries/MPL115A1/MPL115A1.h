//
// MPL115A1 Arduino Library.
//
// Borrowed heavily (and I mean HEAVILY) from test code by J. McConnel and J. Lindblom.
//
// Their headers included below.
//

/*
 MPL115A1 SPI Digital Barometer Test Code
 Created on: September 30, 2010
       By: Jeremiah McConnell - miah at miah.com
 Portions: Jim Lindblom - jim at sparkfun.com
 
 This is a simple test program for the MPL115A1 Pressure Sensor (SPI version).
 
 Hardware: ATmega168, ATmega328
 Powered at 3.3V or 5V, running at 8MHz or 16MHz.
 
 MPL115A1 Breakout ------------- Arduino
 -----------------               -------
         SDN ------------------- D9
         CSN ------------------- D10
         SDO ------------------- D12 *
         SDI ------------------- D11 *
         SCK ------------------- D13 *
         GND ------------------- GND
         VDD ------------------- VCC +
 
         * These pins physically connect to the SPI device so you can't change them
         + 5V board use 5V VDD, 3.3V board use 3.3V VDD
 
 License: CCAv3.0 Attribution-ShareAlike (http://creativecommons.org/licenses/by-sa/3.0/)
 You're free to use this code for any venture, but I'd love to hear about what you do with it,
 and any awesome changes you make to it. Attribution is greatly appreciated.
 */

#include <Arduino.h>

// Pin definitions
#define MPL115A1_ENABLE_PIN 9

// Masks for MPL115A1 SPI i/o
#define MPL115A1_READ_MASK  0x80
#define MPL115A1_WRITE_MASK 0x7F 

// MPL115A1 register address map
#define PRESH   0x00    // 80
#define PRESL   0x02    // 82
#define TEMPH   0x04    // 84
#define TEMPL   0x06    // 86

#define A0MSB   0x08    // 88
#define A0LSB   0x0A    // 8A
#define B1MSB   0x0C    // 8C
#define B1LSB   0x0E    // 8E
#define B2MSB   0x10    // 90
#define B2LSB   0x12    // 92
#define C12MSB  0x14    // 94
#define C12LSB  0x16    // 96
#define C11MSB  0x18    // 98
#define C11LSB  0x1A    // 9A
#define C22MSB  0x1C    // 9C
#define C22LSB  0x1E    // 9E


class MPL115A1
{
public:

    MPL115A1( int selectPin,
              int shutdownPin );

    void writeRegister(byte thisRegister, byte thisValue);
    unsigned int readRegister(byte thisRegister);

    float calculateTemperatureC();
    float calculatePressurekPa();

    int m_selectPin;
    int m_shutdownPin;
};
