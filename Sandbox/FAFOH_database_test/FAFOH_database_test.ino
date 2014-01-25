

// Database username is fafohcom_arduino   <<< INVALID >>>
// Database password is  _shmiFso3QkW
// Database name is fafohcom_arduino1db


#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xCE, 0x24 };

EthernetClient client;
char hostname[]="fafoh.com";

void setup()
{
  Serial.begin(115200);
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    for (;;) ;
  }
  // Give Ethernet shield a second to initialize.
  delay(2000);
  
  
  
  Serial.println("connecting...");
  
  if (client.connect("fafoh.com", 80)) {
    Serial.println("connected");
    client.println("GET /hello.php HTTP/1.1");
    client.print("Host: ");
    client.println(hostname);
    client.println();
  }
  else{
    Serial.println("connection failed");
  }
}

void loop()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    while(true);
  }  
}
