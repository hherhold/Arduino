
#include "MPL115A1.h"
#include "SPI.h"

#define MPL115A1_ENABLE_PIN 9
#define MPL115A1_SELECT_PIN 7

MPL115A1 mpl;


void setup()
{
    // initialize serial i/o
    Serial.begin(9600);
    
    // initialize SPI interface
    SPI.begin();

    
    // these are the defaults
    //SPI.setDataMode(SPI_MODE0);
    //SPI.setClockDivider(SPI_CLOCK_DIV4);  // MPL115A1 supports up to 8MHz
    //SPI.setBitOrder(MSBFIRST);
    
    // initialize the chip select and enable pins
    pinMode(MPL115A1_SELECT_PIN, OUTPUT);
    pinMode(MPL115A1_ENABLE_PIN, OUTPUT);
    
    // sleep the MPL115A1
    digitalWrite(MPL115A1_ENABLE_PIN, LOW);

    delay( 1000 );
    
    // wake the MPL115A1
    digitalWrite(MPL115A1_ENABLE_PIN, HIGH);
    delay(20);  // give the chip a few ms to wake up
    
    // set the chip select inactive, select signal is CS LOW
    digitalWrite(MPL115A1_SELECT_PIN, HIGH);

    mpl.init( MPL115A1_SELECT_PIN, MPL115A1_ENABLE_PIN );
}

void loop()
{
    float pressure_pKa = mpl.calcPressure_kPa();

    Serial.println( pressure_pKa, 4 );

    delay( 2000 );

}

