//
// MPL115A1 Arduino Library.
//

#include <Arduino.h>

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

class MPL115A1
{
public:

    MPL115A1( );

    void init( int selectPin,
               int shutdownPin );

    uint8_t read( uint8_t addr );
    uint8_t write( uint8_t addr, uint8_t value );

    uint32_t getTemp_counts( );
    uint32_t getPress_counts( );

    float calcPressure_kPa( );

    void readCoefficients( );

    int m_selectPin;
    int m_shutdownPin;

    float MPL115A1_coeff_a0;
    float MPL115A1_coeff_b1;
    float MPL115A1_coeff_b2;
    float MPL115A1_coeff_c12;
};
