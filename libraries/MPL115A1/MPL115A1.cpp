
#include "MPL115A1.h"

#include "SPI.h"

MPL115A1::MPL115A1( )
{

}

void MPL115A1::init( int selectPin,
                     int shutdownPin )
{
    SPI.begin();

    m_selectPin = selectPin;
    m_shutdownPin = shutdownPin;

    MPL115A1_coeff_a0  = 0.0;
    MPL115A1_coeff_b1  = 0.0;
    MPL115A1_coeff_b2  = 0.0;
    MPL115A1_coeff_c12 = 0.0;

    readCoefficients( );
}

uint8_t MPL115A1::read( uint8_t addr )
{
    // LSB is don't care, and address is shifted 1 high.
    // MSB is 1 for read, 0 for write.
    addr <<= 1;
    addr |= 0x80;

    digitalWrite( m_selectPin, LOW );
    SPI.transfer( addr );
    uint8_t retVal = SPI.transfer( 0x00 ) & 0xFF;
    digitalWrite( m_selectPin, HIGH );
    return retVal;
}

uint8_t MPL115A1::write( uint8_t addr, uint8_t value )
{
    // LSB is don't care, and address is shifted 1 high.
    // MSB is 1 for read, 0 for write.
    addr <<= 1;

    digitalWrite( m_selectPin, LOW );
    SPI.transfer( addr );
    uint8_t retVal = ( SPI.transfer( value ) ) & 0xFF;
    digitalWrite( m_selectPin, HIGH );
    return retVal;
}

void MPL115A1::readCoefficients( )
{
    // a0
    uint8_t A0_MSB = read( MPL115A1_A0_MSB_RD );
    uint8_t A0_LSB = read( MPL115A1_A0_LSB_RD );
    uint32_t a0bits = ( A0_MSB << 8 ) | ( A0_LSB );

    MPL115A1_coeff_a0 = ( ( a0bits & 0b0111111111111000 ) >> 3 ) +
        ( ( float )( a0bits & 0b0111 ) / ( float )( 0b1000 ) );
//    MPL115A1_coeff_a0 *=  ( a0bits & 0b1000000000000000 ) ? -1.0 : 1.0;
//    Serial.print( "a0 = " ); Serial.println( MPL115A1_coeff_a0, 10 );

    // b1
    //      111 1 11
    //      543 2 1098 7654 3210
    // b1 = SII.F FFFF FFFF FFFF
    uint8_t B1_MSB = read( MPL115A1_B1_MSB_RD );
    uint8_t B1_LSB = read( MPL115A1_B1_LSB_RD );
#if 0
    Serial.print( "B1 msb = 0x" ); Serial.println( B1_MSB, HEX );
    Serial.print( "B1 Lsb = 0x" ); Serial.println( B1_LSB, HEX );
    uint32_t b1bits = ( B1_MSB << 8 ) | ( B1_LSB );
    Serial.print( "B1 bits = 0x" ); Serial.println( b1bits, HEX );
    MPL115A1_coeff_b1 = ( ( ( b1bits & 0b0110000000000000 ) >> 13 )  + 
                          ( float )( b1bits & 0b0001111111111111 ) ) 
        / ( float )( 0b10000000000000 );
//    MPL115A1_coeff_b1 -= 3; // Two's complement notation
//    Serial.print( "b1 = " ); Serial.println( MPL115A1_coeff_b1, 10 );
    MPL115A1_coeff_b1 *= ( b1bits & 0b1000000000000000 ) ? -1.0 : 1.0;
#endif
    // From sparkfun forums
    MPL115A1_coeff_b1 = ( ( ( ( B1_MSB & 0x1F ) * 0x100 ) + B1_LSB ) / 8192.0 ) - 3;
//    Serial.print( "b1 = " ); Serial.println( MPL115A1_coeff_b1, 10 );

    // b2
    uint8_t B2_MSB = read( MPL115A1_B2_MSB_RD );
    uint8_t B2_LSB = read( MPL115A1_B2_LSB_RD );
#if 0
    uint32_t b2bits = ( B2_MSB << 8 ) | ( B2_LSB );
    MPL115A1_coeff_b2 = ( ( b2bits & 0b0100000000000000 ) >> 14 ) +
        ( ( float )( b2bits & 0b001111111111111 ) / ( float )( 0b0100000000000000 ) );
    MPL115A1_coeff_b2 *= ( b2bits & 0b1000000000000000 ) ? -1.0 : 1.0;
#endif
    // From sparkfun forums
    MPL115A1_coeff_b2 = ( ( ( ( B2_MSB - 0x80 ) << 8 ) + B2_LSB ) / 16384.0 ) - 2;
//    Serial.print( "b2 = " ); Serial.println( MPL115A1_coeff_b2, 10 );

    // c12 is the oddball here in that it's 14 bits, with the 2 LSB set to zero.
    uint8_t C12_MSB = read( MPL115A1_C12_MSB_RD );
    uint8_t C12_LSB = read( MPL115A1_C12_LSB_RD );
#if 0
    uint32_t c12bits = ( ( C12_MSB << 8 ) | ( C12_LSB ) ) >> 2;
    MPL115A1_coeff_c12 = 
        ( ( c12bits & 0b01111111111111 ) / ( float )( 0b10000000000000000000000 ) );
    MPL115A1_coeff_c12 *= ( c12bits & 0b10000000000000 ) ? -1.0 : 1.0;
#endif
    // From sparkfun forums
    MPL115A1_coeff_c12 = ( ( ( C12_MSB * 0x100 ) + C12_LSB ) / 16777216.0 );
//    Serial.print( "c12 = " ); Serial.println( MPL115A1_coeff_c12, 10 );
}

uint32_t MPL115A1::getTemp_counts( )
{
    // 10 bit value transferred in MSB of 16 bits.
    uint8_t TEMP_MSB = read( MPL115A1_TEMP_MSB_RD );
    uint8_t TEMP_LSB = read( MPL115A1_TEMP_LSB_RD );
//    Serial.print( "TEMP_MSB = 0x" ); Serial.println( TEMP_MSB, HEX );
//    Serial.print( "TEMP_LSB = 0x" ); Serial.println( TEMP_LSB, HEX );

    uint32_t tempBits = ( ( TEMP_MSB << 8 ) | ( TEMP_LSB ) ) & 0xFFFF;
    tempBits >>= 6;
//    Serial.print( "TEMP = 0x" ); Serial.println( tempBits, HEX );

    return tempBits;
}

uint32_t MPL115A1::getPress_counts( )
{
    // 10 bit value transferred in MSB of 16 bits.
    uint8_t PRESS_MSB = read( MPL115A1_PRESS_MSB_RD );
    uint8_t PRESS_LSB = read( MPL115A1_PRESS_LSB_RD );
    uint32_t pressBits = ( ( PRESS_MSB << 8 ) | ( PRESS_LSB ) ) & 0xFFFF;
    pressBits >>= 6;
    return pressBits;
}


float MPL115A1::calcPressure_kPa( )
{
    write( MPL115A1_CONVERT, 0x00 );
    delay( 5 );

    uint32_t Tadc = getTemp_counts();
    uint32_t Padc = getPress_counts();

//    Serial.print( "Tadc = " ); Serial.println( Tadc );
//    Serial.print( "Padc = " ); Serial.println( Padc );

#if 0
    float c12x2 = MPL115A1_coeff_c12 * ( float )Tadc;
    Serial.print( "c12x2 = " ); Serial.println( c12x2, 8 );
    float a1 = MPL115A1_coeff_b1 + c12x2;
    Serial.print( "a1 = " ); Serial.println( a1, 8 );
    float a1x1 = a1 * ( float )Padc;
    Serial.print( "a1x1 = " ); Serial.println( a1x1, 8 );
    float y1 = MPL115A1_coeff_a0 + a1x1;
    Serial.print( "y1 = " ); Serial.println( y1, 8 );
    float a2x2 = MPL115A1_coeff_b2 * ( float )Tadc;
    Serial.print( "a2x2 = " ); Serial.println( a2x2, 8 );
    float Pcomp = y1 + a2x2;
#endif

    float Pcomp = 
        MPL115A1_coeff_a0 +
        ( MPL115A1_coeff_b1 + ( MPL115A1_coeff_c12 * ( float )( Tadc ))) * Padc +
        MPL115A1_coeff_b2 * ( float )( Tadc );

//    Serial.print( "Pcomp = " ); Serial.println( Pcomp, 10 );

    float press_kPa = Pcomp * ( ( 115.0 - 50.0 ) / 1023.0 ) + 50.0;

    return press_kPa;
}

