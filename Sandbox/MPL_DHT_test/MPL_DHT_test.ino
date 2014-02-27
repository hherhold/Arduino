
#include <SPI.h>
#include <MPL115A1.h>
#include <DHT.h>



#define MPL115A1_ENABLE_PIN 9
#define MPL115A1_SELECT_PIN 7
#define DHT22_PIN 6

dht DHT;
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

#if 0
    Serial.begin(9600);

    SPI.begin();

    pinMode(MPL115A1_SELECT_PIN, OUTPUT);
    pinMode(MPL115A1_ENABLE_PIN, OUTPUT);
    
    mpl.init( MPL115A1_SELECT_PIN, MPL115A1_ENABLE_PIN );    
    digitalWrite(MPL115A1_ENABLE_PIN, LOW);
    digitalWrite(MPL115A1_SELECT_PIN, HIGH);
#endif
}

void loop()
{
    digitalWrite(MPL115A1_ENABLE_PIN, HIGH);
    delay(20);  // give the chip a few ms to wake up
    
    float pressure_pKa = mpl.calcPressure_kPa();

    Serial.print( "MPL115A1: Press (kPa) = " );
    Serial.print( pressure_pKa, 4 );

    // READ DATA
    Serial.print("\tDHT22 ");
    int chk = DHT.read22(DHT22_PIN);
    switch (chk)
    {
    case DHTLIB_OK:  
        Serial.print("read OK,\t"); 
        break;
    case DHTLIB_ERROR_CHECKSUM: 
        Serial.print("Checksum error,\t"); 
        break;
    case DHTLIB_ERROR_TIMEOUT: 
        Serial.print("Time out error,\t"); 
        break;
    default: 
        Serial.print("Unknown error,\t"); 
        break;
    }
    // DISPLAY DATA
    Serial.print( " humidity = " );
    Serial.print(DHT.humidity, 1);
    Serial.print(",\tTemp c = ");
    Serial.println(DHT.temperature, 1);

    delay( 2000 );

}

