//
// MPL115A1 Barometric Pressure Sensor testing
//

#include <SPI.h>

// Pins:
// SCK -> D13
// SDI -> D11 (MOSI)
// SDO -> D12 (MISO)
// CS  -> D7

#define MPL115A1_CS_PIN 7



#define MPL115A1_PRESS_MSB_RD        0x00
#define MPL115A1_PRESS_LSB_RD        0x01
#define MPL115A1_TEMP_MSB_RD         0x02
#define MPL115A1_TEMP_LSB_RD         0x03
#define MPL115A1_A0_MSB_RD           0x04
#define MPL115A1_A0_LSB_RD           0x05
#define MPL115A1_B1_MSB_RD           0x06
#define MPL115A1_B1_LSB_RD           0x07
#define MPL115A1_B2_MSB_RD           0x08
#define MPL115A1_B2_LSB_RD           0x09
#define MPL115A1_C12_MSB_RD          0x0A
#define MPL115A1_C12_LSB_RD          0x0B
#define MPL115A1_CONVERT             0x12

uint8_t MPL115A1_read( uint8_t addr )
{
    // LSB is don't care, and address is shifted 1 high.
    // MSB is 1 for read, 0 for write.
    addr <<= 1;
    addr |= 0x80;

    digitalWrite( MPL115A1_CS_PIN, LOW );
    SPI.transfer( addr );
    uint8_t retVal = SPI.transfer( 0x00 ) & 0xFF;
    digitalWrite( MPL115A1_CS_PIN, HIGH );
    return retVal;
}

uint8_t MPL115A1_write( uint8_t addr )
{
    // LSB is don't care, and address is shifted 1 high.
    // MSB is 1 for read, 0 for write.
    addr <<= 1;

    digitalWrite( MPL115A1_CS_PIN, LOW );
    SPI.transfer( addr );
    uint8_t retVal = ( SPI.transfer( 0x00 ) ) & 0xFF;
    digitalWrite( MPL115A1_CS_PIN, HIGH );
    return retVal;
}

float MPL115A1_coeff_a0  = 0.0;
float MPL115A1_coeff_b1  = 0.0;
float MPL115A1_coeff_b2  = 0.0;
float MPL115A1_coeff_c12 = 0.0;

void MPL115A1_readCoefficients( )
{
    // a0
    uint8_t A0_MSB = MPL115A1_read( MPL115A1_A0_MSB_RD );
    uint8_t A0_LSB = MPL115A1_read( MPL115A1_A0_LSB_RD );
    uint32_t a0bits = ( A0_MSB << 8 ) | ( A0_LSB );
    MPL115A1_coeff_a0 = ( ( a0bits & 0b0111111111111000 ) >> 3 ) +
        ( ( float )( a0bits & 0b0111 ) / ( float )( 0b1000 ) );
    MPL115A1_coeff_a0 *=  ( a0bits & 0b1000000000000000 ) ? -1.0 : 1.0;
    Serial.print( "a0 = " ); Serial.println( MPL115A1_coeff_a0, 10 );

    // b1
    uint8_t B1_MSB = MPL115A1_read( MPL115A1_B1_MSB_RD );
    uint8_t B1_LSB = MPL115A1_read( MPL115A1_B1_LSB_RD );
    uint32_t b1bits = ( B1_MSB << 8 ) | ( B1_LSB );
    MPL115A1_coeff_b1 = ( ( b1bits & 0b0110000000000000 ) >> 13 )  +
        ( ( float )( b1bits & 0b0001111111111111 ) / ( float )( 0b10000000000000 ) );
    MPL115A1_coeff_b1 *= ( b1bits & 0b1000000000000000 ) ? -1.0 : 1.0;
    Serial.print( "b1 = " ); Serial.println( MPL115A1_coeff_b1, 10 );

    // b2
    uint8_t B2_MSB = MPL115A1_read( MPL115A1_B2_MSB_RD );
    uint8_t B2_LSB = MPL115A1_read( MPL115A1_B2_LSB_RD );
    uint32_t b2bits = ( B2_MSB << 8 ) | ( B2_LSB );
    MPL115A1_coeff_b2 = ( ( b2bits & 0b0100000000000000 ) >> 14 ) +
        ( ( float )( b2bits & 0b001111111111111 ) / ( float )( 0b0100000000000000 ) );
    MPL115A1_coeff_b2 *= ( b2bits & 0b1000000000000000 ) ? -1.0 : 1.0;
    Serial.print( "b2 = " ); Serial.println( MPL115A1_coeff_b2, 10 );

    // c12
    uint8_t C12_MSB = MPL115A1_read( MPL115A1_C12_MSB_RD );
    uint8_t C12_LSB = MPL115A1_read( MPL115A1_C12_LSB_RD );
    uint32_t c12bits = ( C12_MSB << 8 ) | ( C12_LSB );
    MPL115A1_coeff_c12 = 
        ( ( c12bits & 0b01111111111111 ) / ( float )( 0b10000000000000000000000 ) );
    MPL115A1_coeff_c12 *= ( c12bits & 0b10000000000000 ) ? -1.0 : 1.0;
    Serial.print( "C12 = " ); Serial.println( MPL115A1_coeff_c12, 10 );
}

void setup()
{
    Serial.begin( 9600 );
    delay( 1000 );

    Serial.println( "MPL115A1 testing" );
    pinMode( MPL115A1_CS_PIN, OUTPUT );
    digitalWrite( MPL115A1_CS_PIN, LOW );
    delay( 100 );
    digitalWrite( MPL115A1_CS_PIN, HIGH );

    SPI.begin();

    delay( 1000 );


    MPL115A1_readCoefficients( );

#if 0
    MPL115A1_write( MPL115A1_CONVERT );
    Serial.print( "Temp MSB = " );
    Serial.println( MPL115A1_read( MPL115A1_TEMP_MSB_RD ), HEX );
    Serial.print( "Temp LSB = " );
    Serial.println( MPL115A1_read( MPL115A1_TEMP_LSB_RD ), HEX );
#endif

    
}

void loop()
{

}

