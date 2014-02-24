

import urllib2
import time
import sys
from subprocess import call

## On Linino, need to reset MCU here.

sys.stdout.write( 'Getting coordinator...' )
sys.stdout.flush()
coordinatorAddress = urllib2.urlopen( 'http://arduino.local/arduino/coordinator' ).read()
print( 'done.' )
print( 'Coordinator: ' + coordinatorAddress )


sys.stdout.write( 'Telling Coordinator to do discovery...' )
sys.stdout.flush()
dodisco = urllib2.urlopen( 'http://arduino.local/arduino/dodiscovery' )
time.sleep( 5 )
print( 'done.' )

sys.stdout.write( 'Getting list of nodes...' )
sys.stdout.flush();
nodeString = urllib2.urlopen( 'http://arduino.local/arduino/listnodes' ).read()
print( 'done.' )

nodeAddresses = nodeString.split()

for node in nodeAddresses:
    print node


