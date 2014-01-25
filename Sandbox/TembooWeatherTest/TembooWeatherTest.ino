#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h" // contains Temboo account information, as described below

int numRuns = 1;   // execution count, so this doesn't run forever
int maxRuns = 10;   // maximum number of times the Choreo should be executed

void setup() {
  Serial.begin(9600);
  
  // For debugging, wait until a serial console is connected.
  delay(4000);
  while(!Serial);
  Bridge.begin();
}
void loop()
{
  if (numRuns <= maxRuns) {
    Serial.println("Running GetWeatherByAddress - Run #" + String(numRuns++));
    
    TembooChoreo GetWeatherByAddressChoreo;

    // invoke the Temboo client
    GetWeatherByAddressChoreo.begin();
    
    // set Temboo account credentials
    GetWeatherByAddressChoreo.setAccountName(TEMBOO_ACCOUNT);
    GetWeatherByAddressChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    GetWeatherByAddressChoreo.setAppKey(TEMBOO_APP_KEY);
    
    // set choreo inputs
    GetWeatherByAddressChoreo.addInput("Address", "48073");
    
    // identify choreo to run
    GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");
    
    // run choreo; when results are available, print them to serial
    GetWeatherByAddressChoreo.run();
    
    while(GetWeatherByAddressChoreo.available()) {
      char c = GetWeatherByAddressChoreo.read();
      Serial.print(c);
    }
    GetWeatherByAddressChoreo.close();

  }

  Serial.println("Waiting...");
  delay(30000); // wait 30 seconds between GetWeatherByAddress calls
}
