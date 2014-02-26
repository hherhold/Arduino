/*
 * Bridge testing
 */

#include <Bridge.h>

void setup() 
{
    Bridge.begin();
}

void loop() 
{
    // Bridge stress test from:
    // http://forum.arduino.cc/index.php?topic=201484.msg1484792#msg1484792
    unsigned int len;
    char buffer[96];
    char prevID='-';
    while(true)
    {
        Bridge.get("key",buffer,96);
        if(buffer[0]!=prevID)
        {
            // new message
            delay(250);
            Bridge.put("key2",&buffer[1]);
            prevID=buffer[0];
        }
        delay(250);
    }
}

