//
// BMA180.h
//

#include <Arduino.h>

//
// #defines and such for dealing with the Bosch BMA180 3-axis accelerometer.
//

//
// Adapted and expanded March-April 2012 by Hollister Herhold
// (hollister at fafoh dot com)
//
// Sections and page numbers refer to the Bosch BMA180
// datasheet. While not exactly a page-turner, it is HIGHLY
// RECOMMENDED READING to understand what's going on here.
//


// This code is based on:

/*
Copyright (c) 2010-2011 George Rhoten

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// And all the disclaimers apply to any and all subsequent revisions.


// i2c device address. This is actually changeable, but this is the default.
#define BMA180_DEVICE_ADDR ((byte)0x40)    

// SPI does not use the device address - it uses a HW chip select instead.
// The register interface, however, operates identically regardless of SPI
// or i2c (if you get it all working right).

// Control Registers. Section 7.10, p.46.
#define BMA180_CTRL_REG0_ADDR 0x0D
#define BMA180_CTRL_REG0_EE_W_BIT 0x10

#define BMA180_CTRL_REG3_ADDR 0x21
#define BMA180_CTRL_REG3_ADV_INT_BIT 0x04

// Offsets. MSB is <11:4>, LSB is <3:0>. You should never have to use
// or change these as these values are set in the factory. Google for
// calibration information if you're feeling really adventurous and
// have your BMA180 mounted securely in something VERY level.
#define BMA180_OFFSET_X_MSB_ADDR     0x38
#define BMA180_OFFSET_Y_MSB_ADDR     0x39
#define BMA180_OFFSET_Z_MSB_ADDR     0x3A
#define BMA180_OFFSET_LSB1_ADDR      0x35
#define BMA180_OFFSET_LSB2_ADDR      0x36

// Also contains Mode Config.
#define BMA180_TCO_Z_ADDR 0x30

#define BMA180_BW_TCS_ADDR 0x20
// BW (Bandwidth) is high-order 4 bits.
#define BMA180_BW_MASK      0xF0
#define BMA180_BW_10HZ      0x00
#define BMA180_BW_20HZ      0x10
#define BMA180_BW_40HZ      0x20
#define BMA180_BW_75HZ      0x30
#define BMA180_BW_150HZ     0x40
#define BMA180_BW_300HZ     0x50
#define BMA180_BW_600HZ     0x60
#define BMA180_BW_1200HZ    0x70
#define BMA180_BW_HIGH_PASS 0x80
#define BMA180_BW_BAND_PASS 0x90

// Chip ID. Reading this to verify connectivity and communication with
// the device is a pretty good idea.
#define BMA180_CHIP_ID_ADDR 0x00

// Acceleration and Temperature data. MSB is <13:6>, LSB is packed 8
// bits (<5:0> 0 new_data_flag).
#define BMA180_ACC_X_LSB_ADDR    0x02
#define BMA180_ACC_X_MSB_ADDR    0x03
#define BMA180_ACC_Y_LSB_ADDR    0x04
#define BMA180_ACC_Y_MSB_ADDR    0x05
#define BMA180_ACC_Z_LSB_ADDR    0x06
#define BMA180_ACC_Z_MSB_ADDR    0x07
#define BMA180_TEMP_ADDR         0x08

#define BMA180_NEW_ACCEL_DATA_BIT   0x01

// Range. Section 7.7.1, p. 27.
// Range is 3 bits, ranging from 1g to 16g. The value is stored in the
// offset_lsb1 register, in the high 3 bits of the lower nibble:
// 0x35 ->   | offset_x<3:0> (lsb) | range<2:0> | smp_skip |
#define BMA180_RANGE_1G          ( 0x00 << 1 )
#define BMA180_RANGE_1_5G        ( 0x01 << 1 )
#define BMA180_RANGE_2G          ( 0x02 << 1 )
#define BMA180_RANGE_3G          ( 0x03 << 1 )
#define BMA180_RANGE_4G          ( 0x04 << 1 )
#define BMA180_RANGE_8G          ( 0x05 << 1 )
#define BMA180_RANGE_16G         ( 0x06 << 1 )
#define BMA180_RANGE_MASK        0x0E


// Class definition for Arduino library use.


typedef struct AccelDataPacket
{
  int x;
  int y;
  int z;
  int t;
};

class BMA180
{
 public:
  static int getDeviceID( );

  static void setRange( byte rangeBits );

  static void getAccelerationData( AccelDataPacket* accelData );

  static void getRawAccelRegs( byte* rawAccelRegs );
};


#if 0

Some readings with 16G setting:

F2 D9 FE F1 FD 75
F2 F1 FE E9 FD 95
F2 D9 FE E1 FD 69
F2 E5 FE F5 FD 81
F2 DD FE E5 FD 85
F2 C9 FE E9 FD 71
F2 ED FE E5 FD 8D
F2 F5 FE D9 FD 81
F2 DD FE F5 FD 75
F2 E1 FE F1 FD 7D
F2 E9 FE E1 FD 89
F2 F5 FE ED FD 7D
F2 F1 FE F5 FD 7D
F2 D1 FE D9 FD 85
F2 E5 FE E9 FD 89

Readings at 2G setting:

C2 C1 FC E1 FF 29
C2 75 FC C1 FF 41
C2 B1 FD 29 FF 59
C3 31 FC BD FF 59
C2 95 FC F5 FF D
C2 A5 FC D9 FF 41
C2 F5 FC A5 FF 49
C2 A9 FC 45 FF 39
C3 5 FC 8D FF 29
C2 A1 FC D5 FF 19
C2 F1 FC B5 FF 41
C2 81 FC 9D FF 45
C2 C9 FC A5 FF 25
C3 9 FC F5 FF 49
C2 E5 FC AD FF 29


#endif
